/*
 * Copyright 2026 wafdy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VEHICLECLOUDGATEWAY_STREAM_TAG_H
#define VEHICLECLOUDGATEWAY_STREAM_TAG_H

#pragma once
namespace gateway::services {
struct IStreamTag {
  virtual void Proceed(bool ok) = 0;
  virtual ~IStreamTag() = default;
};
}  // namespace gateway::services

#endif  // VEHICLECLOUDGATEWAY_STREAM_TAG_H
