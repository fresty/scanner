/* Copyright 2016 Carnegie Mellon University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "scanner/eval/evaluator.h"
#include "scanner/util/common.h"
#include "scanner/util/queue.h"
#include "scanner/video/video_decoder.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <nvcuvid.h>

namespace scanner {

///////////////////////////////////////////////////////////////////////////////
/// PeakVideoDecoder
class PeakVideoDecoder : public VideoDecoder {
 public:
  PeakVideoDecoder(int device_id, DeviceType output_type,
                     CUcontext cuda_context);

  ~PeakVideoDecoder();

  void configure(const InputFormat& metadata) override;

  bool feed(const u8* encoded_buffer, size_t encoded_size,
            bool discontinuity = false) override;

  bool discard_frame() override;

  bool get_frame(u8* decoded_buffer, size_t decoded_size) override;

  int decoded_frames_buffered() override;

  void wait_until_frames_copied() override;

 private:
  static int cuvid_handle_video_sequence(void* opaque, CUVIDEOFORMAT* format);

  static int cuvid_handle_picture_decode(void* opaque,
                                         CUVIDPICPARAMS* picparams);

  static int cuvid_handle_picture_display(void* opaque,
                                          CUVIDPARSERDISPINFO* dispinfo);

  int device_id_;
  DeviceType output_type_;
  CUcontext cuda_context_;
  const int max_output_frames_;
  const int max_mapped_frames_;
  std::vector<cudaStream_t> streams_;

  InputFormat metadata_;
  std::vector<char> metadata_packets_;
  CUvideoparser parser_;
  CUvideodecoder decoder_;

  Queue<CUVIDPARSERDISPINFO> frame_queue_;
  std::vector<std::atomic<u8>> frame_in_use_;
  std::vector<CUdeviceptr> mapped_frames_;
  int last_displayed_frame_;
  std::vector<u8> undisplayed_frames_;
  std::vector<u8> invalid_frames_;
  int wait_for_iframe_;
};
}