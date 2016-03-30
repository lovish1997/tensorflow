package tensorflow

import (
	"fmt"
	"io/ioutil"

	"github.com/golang/protobuf/proto"
)

// Session A Session instance lets a caller drive a TensorFlow graph
// computation.
type Session struct {
	session TF_Session
}

// NewSession initializes a new TensorFlow session.
func NewSession() (*Session, error) {
	status := TF_NewStatus()

	return &Session{
		session: TF_NewSession(
			TF_NewSessionOptions(),
			status,
		),
	}, statusToError(status)
}

// Run Runs the operations on the target nodes, or all the operations if not
// targets are specified. the Parameter Input in a dictionary where the key is
// the tensor name on the graph, and the value the Tensor. The parameter
// outputs is used to specify the tensors from the graph to be returned in the
// same order as they occur on the slice.
func (s *Session) Run(inputs map[string]*Tensor, outputs []string, targets []string) ([]*Tensor, error) {
	inputNames := NewStringVector()
	inputValues := NewTensorVector()
	for k, v := range inputs {
		inputValues.Add(v.tensor)
		inputNames.Add(k)
	}
	outputNames := NewStringVector()
	for _, n := range outputs {
		outputNames.Add(n)
	}

	targetNames := NewStringVector()
	for _, n := range targets {
		targetNames.Add(n)
	}

	outputValues := NewTensorVector()
	status := TF_NewStatus()

	TF_Run_wrapper(s.session, inputNames, inputValues, outputNames, outputValues, targetNames, status)

	result := make([]*Tensor, 0, outputValues.Size())
	for i := int64(0); i < outputValues.Size(); i++ {
		result = append(result, &Tensor{
			tensor: outputValues.Get(int(i)),
		})
	}

	return result, statusToError(status)
}

// LoadGraphFromText Loads a Graph as plain text from the file on the specified
// path.
func LoadGraphFromText(path string) (graph *GraphDef, err error) {
	graphStr, err := ioutil.ReadFile(path)
	if err != nil {
		return
	}

	graph = &GraphDef{}
	if err = proto.UnmarshalText(string(graphStr), graph); err != nil {
		return
	}

	return
}

// ExtendGraph Loads the graph definition on the session
func (s *Session) ExtendGraph(graph *GraphDef) error {
	status := TF_NewStatus()
	buf, err := proto.Marshal(graph)
	if err != nil {
		return err
	}
	TF_ExtendGraph(s.session, buf, status)

	return statusToError(status)
}

func statusToError(status TF_Status) error {
	code := TF_GetCode(status)
	message := TF_Message(status)

	if code != 0 {
		return fmt.Errorf("tensorflow: %d: %v", code, message)
	}

	return nil
}
