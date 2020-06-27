// Licensed to the Software Freedom Conservancy (SFC) under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The SFC licenses this file
// to you under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SaveFileCommandHandler.h"
#include "logging.h"


namespace webdriver {

  SaveFileCommandHandler::SaveFileCommandHandler() {
  }

  SaveFileCommandHandler::~SaveFileCommandHandler() {
  }

  void SaveFileCommandHandler::ExecuteInternal(
    const IECommandExecutor& executor,
    const ParametersMap& command_parameters,
    Response* response) {
    LOG(TRACE) << "Entering SaveFileCommandHandler::ExecuteInternal";
    ParametersMap::const_iterator file_parameter_iterator = command_parameters.find("file");
    if (file_parameter_iterator == command_parameters.end()) {
      response->SetErrorResponse(400, "Missing parameter : file");
      return;
    }
    else {
      std::wstring file_in_base64 = StringUtilities::ToWString(file_parameter_iterator->second.asString());
      if (file_in_base64.size() == 0) {
        response->SetErrorResponse(400, "Found zero size file parameter");
        return;
      }
      else {
        DWORD binary_len = 0;
        PBYTE decoded_file = base64_decode(file_in_base64, binary_len);
        const char *decoded_data = reinterpret_cast<const char*>(decoded_file);
        std::wstring temporary_folder = getTemporaryFolder();
        if (temporary_folder.size() < 3) {
          response->SetErrorResponse(400, "Can't find temporary folder");
          return;
        }
        LOG(INFO) << "found temporary folder '" << temporary_folder.c_str() << "'";

        UUID guid;
        RPC_WSTR guid_string = NULL;
        RPC_STATUS status = ::UuidCreate(&guid);
        status = ::UuidToString(&guid, &guid_string);
        wchar_t* cast_guid_string = reinterpret_cast<wchar_t*>(guid_string);

        std::string file_uuid = StringUtilities::ToString(cast_guid_string);

        ::RpcStringFree(&guid_string);


        //std::wstring output_file_path = temporary_folder + L"\\" + cast_guid_string + L"\\" + cast_guid_string + L".txt";
        std::wstring output_file_path = temporary_folder + L"\\" + cast_guid_string + L".txt";
        LOG(INFO) << "starting to write to '" << output_file_path.c_str() << "'";

        std::ofstream output_stream;
        output_stream.open(output_file_path, std::ios::out | std::ios::binary);
        output_stream.write(decoded_data, binary_len);

        output_stream.close();
      }
    }

  }

  PBYTE  SaveFileCommandHandler::base64_decode(std::wstring& file_in_base64, DWORD& binary_len) {
    PTSTR pszBase64 = (PTSTR) file_in_base64.c_str();
    PBYTE binary_file = NULL;
    DWORD size_of_encoded_data = file_in_base64.size();

    DWORD dwSkip = 0, dwFlags = 0;
    if (CryptStringToBinary(pszBase64, size_of_encoded_data, CRYPT_STRING_BASE64, NULL, &binary_len, &dwSkip, &dwFlags)) {
      binary_file = (PBYTE)CoTaskMemAlloc(binary_len);
      if (binary_file) {
        if (!CryptStringToBinary(pszBase64, size_of_encoded_data, CRYPT_STRING_BASE64, binary_file, &binary_len, &dwSkip, &dwFlags)) {
          CoTaskMemFree(binary_file);
        }
      }
    }
    return binary_file;
  }

  std::wstring  SaveFileCommandHandler::getTemporaryFolder() {
    DWORD bufferSize = 65535;
    std::wstring temporary_folder_buffer;
    temporary_folder_buffer.resize(bufferSize);
    LPCTSTR temp_folder = L"TEMP";
    bufferSize = GetEnvironmentVariable(temp_folder, &temporary_folder_buffer[0], bufferSize);
    if (!bufferSize) {
      return L"";
    }
    temporary_folder_buffer.resize(bufferSize);
    return temporary_folder_buffer;
  }
} // namespace webdriver
