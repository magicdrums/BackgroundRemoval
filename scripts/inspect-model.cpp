#include <iostream>
#include <onnxruntime_cxx_api.h>

int main()
{
	Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "inspect");
	Ort::SessionOptions opts;
	Ort::Session session(env, "/usr/share/obs/obs-plugins/obs-background-removal/models/mediapipe.onnx",
			     opts);

	auto inShape = session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
	auto outShape = session.GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();

	std::cout << "input: ";
	for (auto d : inShape)
		std::cout << d << ' ';
	std::cout << "\noutput: ";
	for (auto d : outShape)
		std::cout << d << ' ';
	std::cout << '\n';
	return 0;
}
