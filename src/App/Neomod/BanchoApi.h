#pragma once
// Copyright (c) 2025, kiwec, All rights reserved.

#include "types.h"
#include <string>

namespace BANCHO::Api {
void append_auth_params(std::string& url, std::string user_param = "u", std::string pw_param = "h");
}  // namespace BANCHO::Api
