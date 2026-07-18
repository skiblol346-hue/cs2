#pragma once
#include "../../Dependencies/ImGui/imgui.h"

class CMenu {
public:
    void Render();
    void Toggle();
    void LoadFonts();
    bool HandleInput() const;

    bool bOpened = true;
    int iActiveTab = 1;
    ImFont* LogoFont = nullptr;
    ImFont* IconFont = nullptr;
    ImFont* ESPFont = nullptr;
};

extern CMenu g_Menu;

extern bool p_enable, p_wall, p_name, p_box, p_health, p_armor, p_ammo, p_weapon, p_skeleton;
extern int box_pos, ammo_pos;
extern float box_col[4], ammo_col[4], name_col[4], skel_col[4];
extern bool f_bomb, f_hp, f_ammo, f_rel, f_scope, f_flash, f_dist, f_kit;

extern bool lb_enable, lb_draw_fov, lb_dynamic_fov, lb_only_body, lb_prefer_head, lb_prefer_body, lb_body_lethal;
extern float lb_amount, lb_multiplier, lb_smooth_pitch, lb_smooth_yaw;
extern float lb_target_switch, lb_min_hit_chance, lb_min_damage;
extern bool hg_selections[6];

extern bool lb_af_enable, lb_af_auto_stop, lb_pro_spread;
extern float lb_delay_shots, lb_af_min_hit, lb_ghost_time;

extern bool vm_hand_shakes, vm_instant_flip, vm_knife_other;
extern float vm_fov;
extern bool vm_custom_origin;
extern float vm_origin_x, vm_origin_y, vm_origin_z;
extern bool vm_custom_angles;
extern float vm_pitch, vm_yaw, vm_roll;

extern bool cam_free_look;
extern float cam_aspect_ratio, cam_fov, cam_zoom_fov1, cam_zoom_fov2;

extern bool tp_enable;
extern float tp_max_dist;