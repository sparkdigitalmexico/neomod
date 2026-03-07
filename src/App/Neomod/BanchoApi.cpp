// Copyright (c) 2025, kiwec, All rights reserved.
#include "BanchoApi.h"
#include "Bancho.h"
#include "fmt/format.h"

namespace BANCHO::Api {

void append_auth_params(std::string &url, std::string user_param, std::string pw_param) {
    std::string user, pw;
    if(BanchoState::is_oauth) {
        user = "$token";
        pw = BanchoState::cho_token;
    } else {
        user = BanchoState::get_username();
        pw = BanchoState::pw_md5.string();
    }

    url.append(fmt::format("&{:s}={:s}&{:s}={:s}", user_param, user, pw_param, pw));
}

}  // namespace BANCHO::Api
