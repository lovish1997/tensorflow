
#include "flatbuffers/flexbuffers.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_custom_options.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.h"

extern "C" {
#include "lib_nn/api/nn_operator.h"
}

namespace tflite {
namespace ops {
namespace micro {
namespace xcore {
namespace bconv {

template <typename Tout>
struct BConv2DArguments {
  Tout *Y;
  const bnn_b32_t *X;
  const bnn_b32_t *K;

  nn_image_params_t x;
  nn_image_params_t y;
  nn_window_params_t k;
};

//**************************************
//**************************************
//**************************************
// BConv2D binary output
//**************************************
//**************************************
//**************************************
namespace bitpacked {

enum class BConv2DKernelType {
  GENERIC,
  DEEPIN,
};

// These are the bconv2d properties args to all threads
struct BConv2DBitpackedArguments : BConv2DArguments<bnn_b32_t> {
  const int32_t *thresholds;
};

struct BConv2DJob : RowColRegion {
  BConv2DJob *next = nullptr;
};

// These are the bconv2d properties unique to each thread
struct BConv2DThreadData {
  BConv2DBitpackedArguments *args;
  BConv2DJob *job;  // This describes the region that that thread will process
  int thread_scratch_idx;
  bnn_b32_t *thread_scratch;  // size should be K_h * K_w * C_in / 32 + 8
};

extern "C" {
ATTRIBUTE_THREAD_FUNCTION void bconv2d_bitpacked_deepin_thread_generator(
    void *context) {
  auto *td = static_cast<BConv2DThreadData *>(context);
  auto *job = td->job;
  while (job) {
    bconv2d_bin_DI_valid(td->args->Y, (bnn_b256_t *)td->args->X,
                         (bnn_b256_t *)td->args->K, td->args->thresholds,
                         &td->args->x, &td->args->y, &td->args->k, job->left,
                         job->top, job->cols, job->rows);
    job = job->next;
  }
}

ATTRIBUTE_THREAD_FUNCTION void bconv2d_bitpacked_thread_generator(
    void *context) {
  auto *td = static_cast<BConv2DThreadData *>(context);
  auto *job = td->job;
  while (job) {
    bconv2d_bin_valid(td->args->Y, td->args->X, td->args->K,
                      td->args->thresholds, td->thread_scratch, &td->args->x,
                      &td->args->y, &td->args->k, job->left, job->top,
                      job->cols, job->rows);
    job = job->next;
  }
}
}

/*
This is a struct that describes the memory required to configure the operator.
*/
struct BConv2DBitpackedOpData {
  // Data that is args to all threads processing the bconv2d
  BConv2DBitpackedArguments args;

  // These are the head pointers to the thread data a thread will have to use.
  BConv2DThreadData *threads;

  // The actual memory used to describe the jobs (regions) threads will have to
  // process.
  BConv2DJob *jobs;

  // The number of concurrent instances of
  // bconv2d_*_thread_generator
  unsigned n_threads;

  // The total number of jobs (regions) processed by the threads, i.e. 6 regions
  // could be processed to 5 threads with 4 threads doing 1 region and one doing
  // 2 regions.
  unsigned n_jobs;

  ExecutionPlan execution_plan;

  size_t stack_size;  // The amount of stack required to run an instance of
                      // bconv2d_*_thread_generator on n_threads threads
  int stack_scratch_index;  // The buffer index where the above stack will be
                            // allocated

  // for loading from external mem
  int weights_scratch_index = 0;
  int bias_scratch_index = 0;
};

void *Init(TfLiteContext *context, const char *buffer, size_t length) {
  auto *op_data = reinterpret_cast<BConv2DBitpackedOpData *>(
      context->AllocatePersistentBuffer(context,
                                        sizeof(BConv2DBitpackedOpData)));

  // parse custom options
  TFLITE_DCHECK(buffer != nullptr);
  TFLITE_DCHECK(length > 0);  // in fact it must be at least 6x uint32_t big

  flexbuffers::Vector Kshape =
      get_named_uint32_custom_option_vector(context, buffer, length, "K");
  op_data->args.y.channels = Kshape[0].AsUInt32();
  op_data->args.k.shape.height = Kshape[1].AsUInt32();
  op_data->args.k.shape.width = Kshape[2].AsUInt32();
  op_data->args.x.channels = Kshape[3].AsUInt32();

  flexbuffers::Vector strides =
      get_named_uint32_custom_option_vector(context, buffer, length, "stride");
  op_data->args.k.stride.vertical = strides[0].AsInt32();
  op_data->args.k.stride.horizontal = strides[1].AsInt32();

  op_data->args.k.dilation.horizontal = 1;
  op_data->args.k.dilation.vertical = 1;

  // TODO
  // parse_custom_options(context, buffer, length, &op_data->execution_plan);
  op_data->n_threads = 1;
  op_data->n_jobs = 1;

  TFLITE_DCHECK(op_data->n_threads > 0);
  TFLITE_DCHECK(op_data->n_jobs > 0);
  TFLITE_DCHECK(op_data->args.k.stride.horizontal > 0);
  TFLITE_DCHECK(op_data->args.k.stride.vertical > 0);
  TFLITE_DCHECK(op_data->args.k.dilation.horizontal > 0);
  TFLITE_DCHECK(op_data->args.k.dilation.vertical > 0);

  // Allocate the jobs (one pointer per thread)
  op_data->threads =
      reinterpret_cast<BConv2DThreadData *>(context->AllocatePersistentBuffer(
          context, sizeof(BConv2DThreadData) * op_data->n_threads));

  // Allocate the jobs (one BConv2DJob per region)
  op_data->jobs =
      reinterpret_cast<BConv2DJob *>(context->AllocatePersistentBuffer(
          context, sizeof(BConv2DJob) * op_data->n_jobs));

  // TODO: this will need the parsed parallelizaiton plan when available
  auto &job = op_data->jobs[0];
  job.top = 0;
  job.left = 0;
  auto &td = op_data->threads[0];
  td.job = &job;
  td.args = &op_data->args;

  return op_data;
}

template <BConv2DKernelType kernel_type>
TfLiteStatus Prepare(TfLiteContext *context, TfLiteNode *node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 3);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  auto *op_data = reinterpret_cast<BConv2DBitpackedOpData *>(node->user_data);

  const TfLiteTensor *input = GetInput(context, node, 0);
  const TfLiteTensor *kernel = GetInput(context, node, 1);
  const TfLiteTensor *thresholds = GetInput(context, node, 2);
  const TfLiteTensor *output = GetOutput(context, node, 0);

  // setup runtime parameters
  op_data->args.x.height = (uint32_t)input->dims->data[1];
  op_data->args.x.width = (uint32_t)input->dims->data[2];
  // FIXME *32
  TF_LITE_ENSURE_EQ(context, op_data->args.x.channels,
                    (uint32_t)input->dims->data[3] * 32);

  op_data->args.y.height = (uint32_t)output->dims->data[1];
  op_data->args.y.width = (uint32_t)output->dims->data[2];
  // FIXME *32
  TF_LITE_ENSURE_EQ(context, op_data->args.y.channels,
                    (uint32_t)output->dims->data[3] * 32);

  // TODO: remove this when parallelization is done
  op_data->jobs[0].cols = op_data->args.y.width;
  op_data->jobs[0].rows = op_data->args.y.height;

  // TODO: fix this this when parallelization is done
  // allocate scratch buffers for weights and biases (if necessary)
  if (!is_ram_address((uintptr_t)kernel->data.data)) {
    TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
        context, kernel->bytes, &op_data->weights_scratch_index));
  }
  if (!is_ram_address((uintptr_t)thresholds->data.data)) {
    TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
        context, thresholds->bytes, &op_data->bias_scratch_index));
  }

  // allocate the stack for thread workers
  if (kernel_type == BConv2DKernelType::DEEPIN) {
    GET_THREAD_FUNCTION_STACKSIZE(op_data->stack_size,
                                  bconv2d_bitpacked_deepin_thread_generator);
  } else if (kernel_type == BConv2DKernelType::GENERIC) {
    GET_THREAD_FUNCTION_STACKSIZE(op_data->stack_size,
                                  bconv2d_bitpacked_thread_generator);
    int thread_scratch_size =
        4 * (op_data->args.k.shape.height * op_data->args.k.shape.width *
                 op_data->args.x.channels / 32 +
             8);  // FIXME *32
    for (int thread_idx = 0; thread_idx < op_data->n_threads; thread_idx++) {
      TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
          context, thread_scratch_size,
          &op_data->threads[thread_idx].thread_scratch_idx));
    }

  } else {
    TF_LITE_KERNEL_LOG(context, "Kernel type not supported by BConv2D.");
    return kTfLiteError;
  }

  TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
      context, op_data->stack_size * op_data->n_threads,
      &op_data->stack_scratch_index));

  return kTfLiteOk;
}

TfLiteStatus EvalCommon(TfLiteContext *context, TfLiteNode *node) {
  auto *op_data = reinterpret_cast<BConv2DBitpackedOpData *>(node->user_data);

  Dispatcher *dispatcher = GetDispatcher();

  const TfLiteTensor *input = GetInput(context, node, 0);
  const TfLiteTensor *kernel = GetInput(context, node, 1);
  const TfLiteTensor *thresholds = GetInput(context, node, 2);
  TfLiteTensor *output = GetOutput(context, node, 0);

  // setup runtime pointers
  op_data->args.X = GetTensorData<bnn_b32_t>(input);
  op_data->args.Y = GetTensorData<bnn_b32_t>(output);

  // load weights & bias scratch buffers (if necessary)
  if (op_data->weights_scratch_index >= 0) {
    op_data->args.K = static_cast<const bnn_b32_t *>(
        context->GetScratchBuffer(context, op_data->weights_scratch_index));
    dispatcher->FetchBuffer((int8_t **)&op_data->args.K,
                            GetTensorData<int8_t>(kernel), kernel->bytes);
  } else {
    op_data->args.K = GetTensorData<bnn_b32_t>(kernel);
  }
  TF_LITE_ENSURE(context, op_data->args.K);

  if (op_data->bias_scratch_index >= 0) {
    op_data->args.thresholds = static_cast<const int32_t *>(
        context->GetScratchBuffer(context, op_data->bias_scratch_index));
    dispatcher->FetchBuffer((int8_t **)&op_data->args.thresholds,
                            GetTensorData<int8_t>(thresholds),
                            thresholds->bytes);
  } else {
    op_data->args.thresholds = GetTensorData<int32_t>(thresholds);
  }
  TF_LITE_ENSURE(context, op_data->args.thresholds);

  return kTfLiteOk;
}

template <BConv2DKernelType kernel_type>
TfLiteStatus Eval(TfLiteContext *context, TfLiteNode *node) {
  TF_LITE_ENSURE_STATUS(EvalCommon(context, node));

  auto *op_data = reinterpret_cast<BConv2DBitpackedOpData *>(node->user_data);

  // initialize the dispatcher
  Dispatcher *dispatcher = GetDispatcher();
  char *stack = static_cast<char *>(
      context->GetScratchBuffer(context, op_data->stack_scratch_index));
  TF_LITE_ENSURE(context, stack);

  if (kernel_type == BConv2DKernelType::DEEPIN) {
    dispatcher->InitializeTasks(bconv2d_bitpacked_deepin_thread_generator,
                                stack, op_data->stack_size);
  } else if (kernel_type == BConv2DKernelType::GENERIC) {
    dispatcher->InitializeTasks(bconv2d_bitpacked_thread_generator, stack,
                                op_data->stack_size);
  } else {
    TF_LITE_KERNEL_LOG(context, "Kernel type not supported by BConv2D.");
    return kTfLiteError;
  }

  // add tasks
  for (int thread_idx = 0; thread_idx < op_data->n_threads; thread_idx++) {
    auto &thread = op_data->threads[thread_idx];
    if (kernel_type == BConv2DKernelType::GENERIC) {
      thread.thread_scratch = static_cast<bnn_b32_t *>(
          context->GetScratchBuffer(context, thread.thread_scratch_idx));
    }
    dispatcher->AddTask(reinterpret_cast<void *>(&thread));
  }

  // start and wait for tasks to complete
  dispatcher->JoinTasks();

  return kTfLiteOk;
}

}  // namespace bitpacked

}  // namespace bconv

TfLiteRegistration *Register_BConv2D_Bitpacked_Deepin() {
  static TfLiteRegistration r = {
      bconv::bitpacked::Init, nullptr,
      bconv::bitpacked::Prepare<bconv::bitpacked::BConv2DKernelType::DEEPIN>,
      bconv::bitpacked::Eval<bconv::bitpacked::BConv2DKernelType::DEEPIN>};
  return &r;
}

TfLiteRegistration *Register_BConv2D_Bitpacked() {
  static TfLiteRegistration r = {
      bconv::bitpacked::Init, nullptr,
      bconv::bitpacked::Prepare<bconv::bitpacked::BConv2DKernelType::GENERIC>,
      bconv::bitpacked::Eval<bconv::bitpacked::BConv2DKernelType::GENERIC>};
  return &r;
}

}  // namespace xcore
}  // namespace micro
}  // namespace ops
}  // namespace tflite
