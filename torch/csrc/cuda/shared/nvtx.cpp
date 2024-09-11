#ifdef _WIN32
#include <wchar.h> // _wgetenv for nvtx
#endif
#include <cuda_runtime.h>
#include <nvToolsExt.h>
#include <torch/csrc/utils/pybind.h>

namespace torch::cuda::shared {

struct RangeHandle {
  nvtxRangeId_t id;
  const char* msg;
};

static void flex_callback_range_end(void* userData) {
  RangeHandle* handle = ((RangeHandle*)userData);
  nvtxRangeEnd(handle->id);
  free((void*)handle->msg);
  free((void*)handle);
}

static void flex_nvtxRangeEnd(void* handle) {
  cudaLaunchHostFunc(0, flex_callback_range_end, handle);
}

static void flex_callback_range_start(void* userData) {
  RangeHandle* handle = ((RangeHandle*)userData);
  handle->id = nvtxRangeStartA(handle->msg);
}

static void* flex_nvtxRangeStart(const char* msg) {
  RangeHandle* handle = (RangeHandle*)calloc(sizeof(RangeHandle), 1);
  handle->msg = strdup(msg);
  handle->id = 0;
  cudaLaunchHostFunc(0, flex_callback_range_start, (void*)handle);
  return handle;
}

void initNvtxBindings(PyObject* module) {
  auto m = py::handle(module).cast<py::module>();

  auto nvtx = m.def_submodule("_nvtx", "libNvToolsExt.so bindings");
  nvtx.def("rangePushA", nvtxRangePushA);
  nvtx.def("rangePop", nvtxRangePop);
  nvtx.def("rangeStartA", nvtxRangeStartA);
  nvtx.def("rangeEnd", nvtxRangeEnd);
  nvtx.def("markA", nvtxMarkA);
  nvtx.def("flex_rangeStart", flex_nvtxRangeStart);
  nvtx.def("flex_rangeEnd", flex_nvtxRangeEnd);
}

} // namespace torch::cuda::shared
