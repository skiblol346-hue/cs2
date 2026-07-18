#pragma once

class c_user_cmd;

struct GameFrameContext {
    c_user_cmd* m_user_cmd = nullptr;
    void* m_local_pawn = nullptr;
    void* m_local_controller = nullptr;
};
