#include "Client/main.h"
#include "Game.h"
#include "apex_sky.h"
#include "vector.h"
#include <array>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib> // For the system() function
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map> // Include the unordered_map header
#include <vector>
// this is a test, with seconds
Memory apex_mem;

// Just setting things up, dont edit.
bool active = true;
uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
uintptr_t locked_aim_entity = 0;
float aiming_score_max;
bool aimbot_safety = true;
int team_player = 0;
const int toRead = 100;
bool aiming = false;
float max_fov = 10;
bool trigger_ready = false;
extern Vector aim_target; // for esp
int map_testing_local_team = 0;

// Removed but not all the way, dont edit.
int glowtype;
int glowtype2;
// float triggerdist = 50.0f;
bool actions_t = false;
bool cactions_t = false;
bool updateInsideValue_t = false;
bool TriggerBotRun_t = false;
bool terminal_t = false;
bool overlay_t = false;
bool esp_t = false;
bool aim_t = false;
bool vars_t = false;
bool item_t = false;
uint64_t g_Base;
bool next2 = false;
bool valid = false;
bool lock = false;
extern float bulletspeed;
extern float bulletgrav;
Vector esp_local_pos;
int local_held_id = 2147483647;
uint32_t local_weapon_id = 2147483647;
int playerentcount = 61;
int itementcount = 10000;
int map = 0;
std::vector<TreasureClue> treasure_clues;

//^^ Don't EDIT^^

// [del]CONFIG AREA, you must set all the true/false to what you want.[/del]
// No longer needed here. Edit your configuration file!

std::vector<uint64_t> wish_list{191, 209, 210, 220,          234,
                                242, 258, 260, 429496729795, 52776987629977800};

void TriggerBotRun() {
  // testing
  // apex_mem.Write<int>(g_Base + OFFSET_IN_ATTACK + 0x8, 4);
  // std::this_thread::sleep_for(std::chrono::milliseconds(10));
  apex_mem.Write<int>(g_Base + OFFSET_IN_ATTACK + 0x8, 5);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  apex_mem.Write<int>(g_Base + OFFSET_IN_ATTACK + 0x8, 4);
  // printf("TriggerBotRun\n");
}
bool IsInCrossHair(Entity &target) {
  static uintptr_t last_t = 0;
  static float last_crosshair_target_time = -1.f;
  float now_crosshair_target_time = target.lastCrossHairTime();
  bool is_trigger = false;
  if (last_t == target.ptr) {
    if (last_crosshair_target_time != -1.f) {
      if (now_crosshair_target_time > last_crosshair_target_time) {
        is_trigger = true;
        // printf("Trigger\n");
        last_crosshair_target_time = -1.f;
      } else {
        is_trigger = false;
        last_crosshair_target_time = now_crosshair_target_time;
      }
    } else {
      is_trigger = false;
      last_crosshair_target_time = now_crosshair_target_time;
    }
  } else {
    last_t = target.ptr;
    last_crosshair_target_time = -1.f;
  }
  return is_trigger;
}

// Used to change things on a timer
/* unsigned char insidevalueItem = 1;
void updateInsideValue()
{
        updateInsideValue_t = true;
        while (updateInsideValue_t)
        {
                insidevalueItem++;
                insidevalueItem %= 256;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                printf("smooth: %f\n", smooth);
                printf("bone: %i\n", bone);
                printf("glowrnot: %f\n", glowrnot);
                printf("glowgnot: %f\n", glowgnot);
                printf("glowbnot: %f\n", glowbnot);


        }
        updateInsideValue_t = false;
} */

// Visual check and aim check.?
float lastvis_esp[toRead];
float lastvis_aim[toRead];
int tmp_spec = 0, spectators = 0;
int tmp_all_spec = 0, allied_spectators = 0;

// works
void SetPlayerGlow(Entity &LPlayer, Entity &Target, int index) {
  const auto g_settings = global_settings();
  int context_id = 0;
  int setting_index = 0;
  std::array<float, 3> highlight_parameter = {0, 0, 0};

  if (!Target.isGlowing() ||
      (int)Target.buffer[OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE] != 1) {
    float currentEntityTime = 5000.f;
    if (!isnan(currentEntityTime) && currentEntityTime > 0.f) {
      if (!(g_settings.firing_range) &&
          (Target.isKnocked() || !Target.isAlive())) {
        context_id = 5;
        setting_index = 80;
        highlight_parameter = {g_settings.glow_r_knocked,
                               g_settings.glow_g_knocked,
                               g_settings.glow_b_knocked};
      } else if (Target.lastVisTime() > lastvis_aim[index] ||
                 (Target.lastVisTime() < 0.f && lastvis_aim[index] > 0.f)) {
        context_id = 6;
        setting_index = 81;
        highlight_parameter = {g_settings.glow_r_viz, g_settings.glow_g_viz,
                               g_settings.glow_b_viz};
      } else {
        if (g_settings.player_glow_armor_color) {
          int shield = Target.getShield();
          int health = Target.getHealth();
          if (shield + health <= 100) { // Orange
            setting_index = 91;
            highlight_parameter = {255 / 255.0, 165 / 255.0, 0 / 255.0};
          } else if (shield + health <= 150) { // white
            setting_index = 92;
            highlight_parameter = {247 / 255.0, 247 / 255.0, 247 / 255.0};
          } else if (shield + health <= 175) { // blue
            setting_index = 93;
            highlight_parameter = {39 / 255.0, 178 / 255.0, 255 / 255.0};
          } else if (shield + health <= 200) { // purple
            setting_index = 94;
            highlight_parameter = {206 / 255.0, 59 / 255.0, 255 / 255.0};
          } else if (shield + health <= 225) { // red
            setting_index = 95;
            highlight_parameter = {219 / 255.0, 2 / 255.0, 2 / 255.0};
          } else {
            setting_index = 90;
            highlight_parameter = {2 / 255.0, 2 / 255.0, 2 / 255.0};
          }
          //   switch (shield_level) {
          //   case 1: // white
          //     setting_index = 91;
          //     highlightParameter = {247 / 255.0, 247 / 255.0, 247 / 255.0};
          //     break;
          //   case 2: // blue
          //     setting_index = 92;
          //     highlightParameter = {39 / 255.0, 178 / 255.0, 255 / 255.0};
          //     break;
          //   case 3: // purple
          //     setting_index = 93;
          //     highlightParameter = {206 / 255.0, 59 / 255.0, 255 / 255.0};
          //     break;
          //   case 4: // gold
          //     setting_index = 94;
          //     highlightParameter = {255 / 255.0, 255 / 255.0, 79 / 255.0};
          //     break;
          //   case 5: // red
          //     setting_index = 95;
          //     highlightParameter = {219 / 255.0, 2 / 255.0, 2 / 255.0};
          //     break;
          //   default:
          //     setting_index = 90;
          //     highlightParameter = {2 / 255.0, 2 / 255.0, 2 / 255.0};
          //   }
        } else {
          context_id = 7;
          setting_index = 82;
          highlight_parameter = {g_settings.glow_r_not, g_settings.glow_g_not,
                                 g_settings.glow_b_not};
        }
      }
      if (g_settings.player_glow) {
        Target.enableGlow(
            context_id, setting_index, g_settings.player_glow_inside_value,
            g_settings.player_glow_outline_size, highlight_parameter);
      } else {
        Target.enableGlow(context_id, setting_index, 0, 0, highlight_parameter);
      }
    }
  }
}

void MapRadarTesting() {
  uintptr_t pLocal;
  apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, pLocal);
  int dt;
  apex_mem.Read<int>(pLocal + OFFSET_TEAM, dt);
  map_testing_local_team = dt;

  for (uintptr_t i = 0; i <= 80000; i++) {
    apex_mem.Write<int>(pLocal + OFFSET_TEAM, 1);
  }

  for (uintptr_t i = 0; i <= 80000; i++) {
    apex_mem.Write<int>(pLocal + OFFSET_TEAM, dt);
  }
  map_testing_local_team = 0;
}

uint64_t PlayerLocal;
int PlayerLocalTeamID;
int EntTeam;
int LocTeam;

std::chrono::steady_clock::time_point tduckStartTime;
bool mapRadarTestingEnabled = true;

uint32_t button_state[4];
bool isPressed(uint32_t button_code) {
  return (button_state[static_cast<uint32_t>(button_code) >> 5] &
          (1 << (static_cast<uint32_t>(button_code) & 0x1f))) != 0;
}

void memory_io_panic(const char *info) {
  quit_tui_menu();
  std::cout << "Error " << info << std::endl;
  exit(0);
}

// Define rainbow color function
void rainbowColor(int frame_number, std::array<float, 3> &colors) {
  const float frequency = 0.1; // Adjust the speed of color change
  const float amplitude = 0.5; // Adjust the amplitude of color change

  // Use the sine function to generate rainbow color variation
  float r = sin(frequency * frame_number + 0) * amplitude + 0.5;
  float g = sin(frequency * frame_number + 2) * amplitude + 0.5;
  float b = sin(frequency * frame_number + 4) * amplitude + 0.5;

  // Clamp the colors to the range [0, 1]
  colors[0] = fmax(0, fmin(1, r));
  colors[1] = fmax(0, fmin(1, g));
  colors[2] = fmax(0, fmin(1, b));
}

void ClientActions() {
  cactions_t = true;
  while (cactions_t) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    while (g_Base != 0) {
      const auto g_settings = global_settings();

      // read player ptr
      uint64_t local_player_ptr = 0;
      apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, local_player_ptr);

      // read game states
      apex_mem.Read<typeof(button_state)>(g_Base + OFFSET_INPUT_SYSTEM + 0xb0,
                                          button_state);

      int attack_state = 0, zoom_state = 0, tduck_state = 0, jump_state = 0,
          force_jump = 0, force_toggle_duck = 0, force_duck = 0,
          curFrameNumber = 0;
      apex_mem.Read<int>(g_Base + OFFSET_IN_ATTACK, attack_state);     // 108
      apex_mem.Read<int>(g_Base + OFFSET_IN_ZOOM, zoom_state);         // 109
      apex_mem.Read<int>(g_Base + OFFSET_IN_TOGGLE_DUCK, tduck_state); // 61
      apex_mem.Read<int>(g_Base + OFFSET_IN_JUMP, jump_state);
      apex_mem.Read<int>(g_Base + OFFSET_IN_JUMP + 0x8, force_jump);
      apex_mem.Read<int>(g_Base + OFFSET_IN_TOGGLE_DUCK + 0x8,
                         force_toggle_duck);
      apex_mem.Read<int>(g_Base + OFFSET_IN_DUCK + 0x8, force_duck);
      apex_mem.Read<int>(g_Base + OFFSET_GLOBAL_VARS + 0x0008,
                         curFrameNumber); // GlobalVars + 0x0008

      float world_time, traversal_start_time, traversal_progress;
      if (!apex_mem.Read<float>(local_player_ptr + OFFSET_TIME_BASE,
                                world_time)) {
        // memory_io_panic("read time_base");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        break;
      }
      if (!apex_mem.Read<float>(local_player_ptr + OFFSET_TRAVERSAL_STARTTIME,
                                traversal_start_time)) {
        // memory_io_panic("read traversal_starttime");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        break;
      }
      if (!apex_mem.Read<float>(local_player_ptr + OFFSET_TRAVERSAL_PROGRESS,
                                traversal_progress)) {
        memory_io_panic("read traversal_progress");
      }

      //   printf("Travel Time: %f\n", traversal_progress);
      //   printf("Cur Frame: %i\n", curFrameNumber);
      //   printf("Jump Value: %i\n", jump_state);
      //   printf("Jump Value: %i\n", force_jump);
      //   printf("ToggleDuck Value: %i\n", force_toggle_duck);
      //   printf("Duck Value: %i\n", force_duck);

      if (g_settings.super_key_toggle) {
        /** SuperGlide
         * https://www.unknowncheats.me/forum/apex-legends/578160-external-auto-superglide-3.html
         */
        float hang_on_wall = world_time - traversal_start_time;

        static float start_jump_time = 0;
        static bool start_sg = false;
        static std::chrono::milliseconds last_sg_finish;

        float hang_start, hang_cancel, trav_start, hang_max, action_interval;
        int release_wait;
        {
          // for 75 fps
          hang_start = 0.1;
          hang_cancel = 0.12;
          trav_start = 0.87;
          hang_max = 1.5;
          action_interval = 0.011;
          release_wait = 50;
          if (abs(g_settings.game_fps - 144.0) <
              abs(g_settings.game_fps - 75.0)) {
            // for 144 fps
            hang_start = 0.05;
            hang_cancel = 0.07;
            trav_start = 0.90;
            hang_max = 0.75;
            action_interval = 0.007;
            release_wait = 35;
            if (abs(g_settings.game_fps - 240.0) <
                abs(g_settings.game_fps - 144.0)) {
              // for 240 fps
              hang_start = 0.033;
              hang_cancel = 0.04;
              trav_start = 0.95;
              hang_max = 0.2;
              action_interval = 0.004;
              release_wait = 20;
            }
          }
        }

        if (hang_on_wall > hang_start) {
          if (hang_on_wall < hang_cancel) {
            apex_mem.Write<int>(g_Base + OFFSET_IN_JUMP + 0x8, 4);
          }
          if (traversal_progress > trav_start && hang_on_wall < hang_max &&
              !start_sg) {
            std::chrono::milliseconds now_ms =
                duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
            if ((now_ms - last_sg_finish).count() > 320 && jump_state > 0) {
              // start SG
              start_jump_time = world_time;
              start_sg = true;
            }
          }
        }
        if (start_sg) {
          // press button
          // g_logger += "sg Press jump\n";
          apex_mem.Write<int>(g_Base + OFFSET_IN_JUMP + 0x8, 5);

          float current_time;
          while (true) {
            if (apex_mem.Read<float>(local_player_ptr + OFFSET_TIME_BASE,
                                     current_time)) {
              if (current_time - start_jump_time < action_interval) {
                // keep looping
              } else {
                break;
              }
            }
          }
          apex_mem.Write<int>(g_Base + OFFSET_IN_DUCK + 0x8, 6);
          std::this_thread::sleep_for(std::chrono::milliseconds(release_wait));
          apex_mem.Write<int>(g_Base + OFFSET_IN_JUMP + 0x8, 4);
          // Write<int>(g_Base + OFFSET_IN_DUCK + 0x8, 4);
          last_sg_finish = duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch());
          // g_logger += "sg\n";
          start_sg = false;
        }
      }

      { /* calc game fps */
        static int last_checkpoint_frame = 0;
        static std::chrono::milliseconds checkpoint_time;
        if (g_settings.calc_game_fps && curFrameNumber % 100 == 0) {
          std::chrono::milliseconds ms =
              duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch());
          int delta_frame = curFrameNumber - last_checkpoint_frame;
          if (delta_frame > 90 && delta_frame < 120) {
            auto duration = ms - checkpoint_time;
            auto settings_state = g_settings;
            settings_state.game_fps = delta_frame * 1000.0f / duration.count();
            update_settings(settings_state);
          }
          last_checkpoint_frame = curFrameNumber;
          checkpoint_time = ms;
        }
      }

      // printf("Minimap: %ld\n", minimap);
      // apex_mem.Write(LocalPlayer + 0x270 , 1);

      /*
      108 Left mouse button (mouse1)
      109 Right mouse button (mouse2)
      110 Middle mouse button (mouse3)
      111 Side mouse button (mouse4)
      112 Side mouse button (mouse5)

      79 SHIFT key
      81 ALT key
      83 CTRL key

      1 KEY_0
      2 KEY_1
      3 KEY_2
      4 KEY_3
      5 KEY_4
      6 KEY_5
      7 KEY_6
      8 KEY_7
      9 KEY_8
      10 KEY_9

      11 KEY_A
      12 KEY_B
      13 KEY_C
      14 KEY_D
      15 KEY_E
      16 KEY_F
      17 KEY_G
      18 KEY_H
      19 KEY_I
      20 KEY_J
      21 KEY_K
      22 KEY_L
      23 KEY_M
      24 KEY_N
      25 KEY_O
      26 KEY_P
      27 KEY_Q
      28 KEY_R
      29 KEY_S
      30 KEY_T
      31 KEY_U
      32 KEY_V
      33 KEY_W
      34 KEY_X
      35 KEY_Y
      36 KEY_Z


      37 KEY_PAD_0
      38 KEY_PAD_1
      39 KEY_PAD_2
      40 KEY_PAD_3
      41 KEY_PAD_4
      42 KEY_PAD_5
      43 KEY_PAD_6
      44 KEY_PAD_7
      45 KEY_PAD_8
      46 KEY_PAD_9
      47 KEY_PAD_DIVIDE
      48 KEY_PAD_MULTIPLY
      49 KEY_PAD_MINUS
      50 KEY_PAD_PLUS
      51 KEY_PAD_ENTER
      52 KEY_PAD_DECIMAL


      65 KEY_SPACE
      67 KEY_TAB
      68 KEY_CAPSLOCK
      69 KEY_NUMLOCK
      70 KEY_ESCAPE
      71 KEY_SCROLLLOCK
      72 KEY_INSERT
      73 KEY_DELETE
      74 KEY_HOME
      75 KEY_END
      76 KEY_PAGEUP
      77 KEY_PAGEDOWN
      78 KEY_BREAK


      88 KEY_UP
      89 KEY_LEFT
      90 KEY_DOWN
      91 KEY_RIGHT


      92 KEY_F1
      93 KEY_F2
      94 KEY_F3
      95 KEY_F4
      96 KEY_F5
      97 KEY_F6
      98 KEY_F7
      99 KEY_F8
      100 KEY_F9
      101 KEY_F10
      102 KEY_F11
      103 KEY_F12
      */

      /* if (isPressed(79)) //TESTING KEYS
      {
              printf("Shift Pressed\n");
      }
      if (isPressed(81)) //TESTING KEYS
      {
              printf("ALT Pressed\n");
      }
      if (isPressed(83)) //TESTING KEYS
      {
              printf("CTRL Pressed0\n");
      } */

      if (g_settings.keyboard) {
        if (isPressed(g_settings.aimbot_hot_key_1) ||
            isPressed(g_settings.aimbot_hot_key_2) ||
            isPressed(g_settings.trigger_bot_hot_key) &&
            !isPressed(112)) // Left and Right click
        {
          aiming = true;
        } else {
          aiming = false;
        }
        if (isPressed(g_settings.aimbot_hot_key_1) ||
            !isPressed(g_settings.aimbot_hot_key_2) ||
            isPressed(g_settings.trigger_bot_hot_key)) {
          max_fov = g_settings.non_ads_fov;
        }
        if (isPressed(g_settings.aimbot_hot_key_2)) {
          max_fov = g_settings.ads_fov;
        }
        if (g_settings.auto_shoot &&
            isPressed(g_settings.trigger_bot_hot_key)) // Left and Right click
        {
          trigger_ready = true;
        } else {
          trigger_ready = false;
        }
      }

      if (g_settings.gamepad) {
        // attackState == 120 || zoomState == 119
        if (attack_state > 0 || zoom_state > 0) {
          aiming = true;
        } else {
          aiming = false;
        }

        if (zoom_state > 0) {
          max_fov = g_settings.ads_fov;
        } else {
          max_fov = g_settings.non_ads_fov;
        }
      }

      // Toggle crouch = check for ring
      if (g_settings.map_radar_testing && attack_state == 0 &&
          isPressed(99)) { // KEY_F8
        if (mapRadarTestingEnabled) {
          MapRadarTesting();
        }

        if (tduckStartTime == std::chrono::steady_clock::time_point()) {
          tduckStartTime = std::chrono::steady_clock::now();
        }

        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                            currentTime - tduckStartTime)
                            .count();
        if (duration >= 500) {
          mapRadarTestingEnabled = false;
        }
      } else {
        tduckStartTime = std::chrono::steady_clock::time_point();
        mapRadarTestingEnabled = true;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  cactions_t = false;
}

void ProcessPlayer(Entity &LPlayer, Entity &target, uint64_t entitylist,
                   int index) {
  const auto g_settings = global_settings();

  int entity_team = target.getTeamId();

  if (!target.isAlive()) {
    float localyaw = LPlayer.GetYaw();
    float targetyaw = target.GetYaw();

    if (localyaw == targetyaw) {
      if (LPlayer.getTeamId() == entity_team)
        tmp_all_spec++;
      else
        tmp_spec++;
    }
    return;
  }

  if (g_settings.tdm_toggle) { // Check if the target entity is on the same
                               // team as the
                               // local player
    // int entity_team = Target.getTeamId();
    // printf("Target Team: %i\n", entity_team);

    uint64_t PlayerLocal;
    apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, PlayerLocal);
    int PlayerLocalTeamID;
    apex_mem.Read<int>(PlayerLocal + OFFSET_TEAM, PlayerLocalTeamID);

    if (entity_team % 2)
      EntTeam = 1;
    else
      EntTeam = 2;
    if (PlayerLocalTeamID % 2)
      LocTeam = 1;
    else
      LocTeam = 2;

    // printf("Target Team: %i\nLocal Team: %i\n", EntTeam, LocTeam);
    if (EntTeam == LocTeam)
      return;
  }

  // Firing range stuff
  if (!g_settings.firing_range) {
    if (entity_team < 0 || entity_team > 50 ||
        (entity_team == team_player && !g_settings.onevone)) {
      return;
    }
    if (map_testing_local_team != 0 && entity_team == map_testing_local_team) {
      return;
    }
  }

  Vector EntityPosition = target.getPosition();
  Vector LocalPlayerPosition = LPlayer.getPosition();
  float dist = LocalPlayerPosition.DistTo(EntityPosition);

  // aim distance check
  if ((local_held_id == -251 && dist > g_settings.skynade_dist) ||
      dist > g_settings.aim_dist)
    return;

  // Targeting
  const float vis_weights = 12.5f;
  float fov = CalculateFov(LPlayer, target);
  bool vis = target.lastVisTime() > lastvis_aim[index];
  float score =
      (fov * fov) * 100 + (dist * 0.025) * 10 + (vis ? 0 : vis_weights);
  /*
   fov:dist:score
    1  10m  100
    2  40m  400
    3  90m  900
    4  160m 1600
  */
  if (score < aiming_score_max) {
    aiming_score_max = score;
    tmp_aimentity = target.ptr;
  }

  if (g_settings.aim == 2) {
    // vis check
    if (aimentity == target.ptr) {
      if (local_held_id != -251 && !vis) {
        // turn on safety
        aimbot_safety = true;
      } else {
        aimbot_safety = false;
      }
    }

    // TriggerBot
    if (aimentity != 0) {
      uint64_t LocalPlayer = 0;
      apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);

      Entity Target = getEntity(aimentity);
      // Entity LPlayer = getEntity(LocalPlayer);

      if (trigger_ready && IsInCrossHair(Target)) {
        TriggerBotRun();
      }
    }
  }
  SetPlayerGlow(LPlayer, target, index);
  lastvis_aim[index] = target.lastVisTime();
}
std::map<uint64_t, int> centityToNumber; // Map centity to a unique number
int uniqueNumber = 1;                    // Initialize a unique number
// Main stuff, dont edit.
void DoActions() {
  actions_t = true;
  while (actions_t) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    uint32_t counter = 0;

    while (g_Base != 0) {
      const auto g_settings = global_settings();

      char MapName[200] = {0};
      uint64_t MapName_ptr;
      apex_mem.Read<uint64_t>(g_Base + OFFSET_HOST_MAP, MapName_ptr);
      apex_mem.ReadArray<char>(MapName_ptr, MapName, 200);

      // printf("%s\n", MapName);
      if (strcmp(MapName, "mp_rr_tropic_island_mu1_storm") == 0) {
        map = 1;
      } else if (strcmp(MapName, "mp_rr_canyonlands_mu") == 0) {
        map = 2;
      } else if (strcmp(MapName, "mp_rr_desertlands_hu") == 0) {
        map = 3;
      } else if (strcmp(MapName, "mp_rr_olympus") == 0) {
        map = 4;
      } else if (strcmp(MapName, "mp_rr_divided_moon") == 0) {
        map = 5;
      } else {
        map = 0;
      }

      if (g_settings.firing_range) {
        playerentcount = 16000;
      } else {
        playerentcount = 61;
      }
      if (g_settings.deathbox) {
        itementcount = 15000;
      } else {
        itementcount = 10000;
      }
      std::this_thread::sleep_for(
          std::chrono::milliseconds(30)); // don't change xD

      uint64_t LocalPlayer = 0;
      apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
      if (LocalPlayer == 0)
        continue;

      Entity LPlayer = getEntity(LocalPlayer);

      team_player = LPlayer.getTeamId();
      if (team_player < 0 || team_player > 50) {
        continue;
      }
      uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

      uint64_t baseent = 0;
      apex_mem.Read<uint64_t>(entitylist, baseent);
      if (baseent == 0) {
        continue;
      }

      aiming_score_max = (50 * 50) * 100 + (g_settings.aim_dist * 0.025) * 10;
      tmp_aimentity = 0;
      tmp_spec = 0;
      tmp_all_spec = 0;
      if (g_settings.firing_range) {
        int c = 0;
        for (int i = 0; i < playerentcount; i++) {
          uint64_t centity = 0;
          apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
          if (centity == 0)
            continue;
          if (LocalPlayer == centity)
            continue;

          Entity Target = getEntity(centity);
          if (!Target.isDummy() && !g_settings.onevone) {
            continue;
          }

          ProcessPlayer(LPlayer, Target, entitylist, c);
          c++;
        }
      } else {

        for (int i = 0; i < toRead; i++) {
          uint64_t centity = 0;
          apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
          if (centity == 0)
            continue;
          if (LocalPlayer == centity)
            continue;

          Entity Target = getEntity(centity);
          if (!Target.isPlayer()) {
            continue;
          }

          ProcessPlayer(LPlayer, Target, entitylist, i);
        }
      }

      if (!spectators && !allied_spectators) {
        spectators = tmp_spec;
        allied_spectators = tmp_all_spec;
      } else {
        // refresh spectators count every ~2 seconds
        counter++;
        if (counter == 70) {
          spectators = tmp_spec;
          allied_spectators = tmp_all_spec;
          counter = 0;
        }
      }

      // set current aim entity
      if (lock) { // locked target
        aimentity = locked_aim_entity;
      } else { // or new target
        aimentity = tmp_aimentity;
      }

      // weapon model glow
      // printf("%d\n", LPlayer.getHealth());
      if (g_settings.weapon_model_glow && LPlayer.getHealth() > 0) {
        std::array<float, 3> highlight_color;
        if (spectators > 0) {
          highlight_color = {1, 0, 0};
        } else if (allied_spectators > 0) {
          highlight_color = {0, 1, 0};
        } else {
          int frame_number = 0;
          apex_mem.Read<int>(g_Base + OFFSET_GLOBAL_VARS + 0x0008,
                             frame_number);
          rainbowColor(frame_number, highlight_color);
        }
        // printf("R: %f, G: %f, B: %f\n", highlight_color[0],
        // highlight_color[1], highlight_color[2]);
        LPlayer.glow_weapon_model(g_Base, true, highlight_color);
        // LPlayer.enableGlow(5, 199, 14, 32, highlight_color);
      } else {
        LPlayer.glow_weapon_model(g_Base, false, {0, 0, 0});
      }
    }
  }
  actions_t = false;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<player> players(toRead);
Matrix view_matrix_data = {};

// ESP loop.. this helps right?
static void EspLoop() {
  esp_t = true;
  while (esp_t) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    while (g_Base != 0 && overlay_t) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      const auto g_settings = global_settings();

      if (g_settings.esp) {
        valid = false;

        uint64_t LocalPlayer = 0;
        apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
        if (LocalPlayer == 0) {
          next2 = true;
          while (next2 && g_Base != 0 && overlay_t && g_settings.esp) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          continue;
        }
        Entity LPlayer = getEntity(LocalPlayer);
        int team_player = LPlayer.getTeamId();
        if (team_player < 0 || team_player > 50) {
          next2 = true;
          while (next2 && g_Base != 0 && overlay_t && g_settings.esp) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          continue;
        }
        Vector LocalPlayerPosition = LPlayer.getPosition();
        esp_local_pos = LocalPlayerPosition;

        uint64_t viewRenderer = 0;
        apex_mem.Read<uint64_t>(g_Base + OFFSET_RENDER, viewRenderer);
        uint64_t viewMatrix = 0;
        apex_mem.Read<uint64_t>(viewRenderer + OFFSET_MATRIX, viewMatrix);

        apex_mem.Read<Matrix>(viewMatrix, view_matrix_data);

        uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

        players.clear();

        {
          Vector LocalPlayerPosition = LPlayer.getPosition();
          QAngle localviewangle = LPlayer.GetViewAngles();

          // Ammount of ents to loop, dont edit.
          for (int i = 0; i < toRead; i++) {
            // Read entity pointer
            uint64_t centity = 0;
            apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
            if (centity == 0) {
              continue;
            }

            // Exclude self
            if (LocalPlayer == centity) {
              continue;
            }

            // Get entity data
            Entity Target = getEntity(centity);

            // Exclude undesired entity
            if (g_settings.firing_range) {
              if (!Target.isDummy() && !g_settings.onevone) {
                continue;
              }
            } else {
              if (!Target.isPlayer()) {
                continue;
              }
            }

            // Exclude dead entity
            if (!Target.isAlive()) {
              continue;
            }

            int entity_team = Target.getTeamId();

            // Exclude invalid team
            if (entity_team < 0 || entity_team > 50) {
              continue;
            }

            // Exlude teammates if not 1v1
            if (entity_team == team_player && !g_settings.onevone) {
              continue;
            }
            // if (map_testing_local_team != 0 &&
            //     entity_team == map_testing_local_team) {
            //   continue;
            // }

            Vector EntityPosition = Target.getPosition();
            float dist = LocalPlayerPosition.DistTo(EntityPosition);

            // Excluding targets that are too far or too close
            if (dist > g_settings.max_dist || dist < 20.0f) {
              continue;
            }

            Vector bs = Vector();
            // Change res to your res here, default is 1080p but can copy paste
            // 1440p here
            WorldToScreen(EntityPosition, view_matrix_data.matrix,
                          g_settings.screen_width, g_settings.screen_height,
                          bs); // 2560, 1440
            if (g_settings.esp) {
              Vector hs = Vector();
              Vector HeadPosition = Target.getBonePositionByHitbox(0);
              // Change res to your res here, default is 1080p but can copy
              // paste 1440p here
              WorldToScreen(HeadPosition, view_matrix_data.matrix,
                            g_settings.screen_width, g_settings.screen_height,
                            hs); // 2560, 1440
              float height = abs(abs(hs.y) - abs(bs.y));
              float width = height / 2.0f;
              float boxMiddle = bs.x - (width / 2.0f);
              int health = Target.getHealth();
              int shield = Target.getShield();
              int maxshield = Target.getMaxshield();
              int armortype = Target.getArmortype();
              Vector EntityPosition = Target.getPosition();
              float targetyaw = Target.GetYaw();
              player data_buf = {dist,
                                 entity_team,
                                 boxMiddle,
                                 hs.y,
                                 width,
                                 height,
                                 bs.x,
                                 bs.y,
                                 Target.isKnocked(),
                                 (Target.lastVisTime() > lastvis_esp[i]),
                                 health,
                                 shield,
                                 maxshield,
                                 armortype,
                                 EntityPosition,
                                 LocalPlayerPosition,
                                 localviewangle,
                                 targetyaw};
              Target.get_name(g_Base, i - 1, &data_buf.name[0]);
              players.push_back(data_buf);
              lastvis_esp[i] = Target.lastVisTime();
              valid = true;
            }
          }
        }

        next2 = true;
        while (next2 && g_Base != 0 && overlay_t && g_settings.esp) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
    }
  }
  esp_t = false;
}

// Aimbot Loop stuff
inline static void lock_target(uintptr_t target_ptr) {
  lock = true;
  locked_aim_entity = target_ptr;
}
inline static void cancel_targeting() {
  lock = false;
  locked_aim_entity = 0;
}
static void AimbotLoop() {
  aim_t = true;
  while (aim_t) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    while (g_Base != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      const auto g_settings = global_settings();

      // Read LocalPlayer
      uint64_t LocalPlayer = 0;
      apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
      // Read HeldID
      int HeldID;
      apex_mem.Read<int>(LocalPlayer + OFFSET_OFF_WEAPON, HeldID); // 0x1a1c
      local_held_id = HeldID;
      // Read WeaponID
      ulong ehWeaponHandle;
      apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_ACTIVE_WEAPON,
                              ehWeaponHandle); // 0x1a1c
      ehWeaponHandle &= 0xFFFF;                // eHandle
      ulong pWeapon;
      uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
      apex_mem.Read<uint64_t>(entitylist + (ehWeaponHandle * 0x20), pWeapon);
      uint32_t weaponID;
      apex_mem.Read<uint32_t>(pWeapon + OFFSET_WEAPON_NAME,
                              weaponID); // 0x1844
      local_weapon_id = weaponID;
      // printf("%d\n", weaponID);

      if (g_settings.aim > 0) {
        if (aimentity == 0) {
          cancel_targeting();
          continue;
        }

        Entity target = getEntity(aimentity);

        if (!aiming) {
          cancel_targeting();
          // show target indicator before aiming
          aim_target = target.getPosition();
          continue;
        }

        if (aimbot_safety) {
          continue;
        }
        lock_target(aimentity);

        Entity LPlayer = getEntity(LocalPlayer);
        if (LocalPlayer == 0) {
          continue;
        }
        if (LPlayer.isKnocked()) {
          cancel_targeting();
          continue;
        }

        /* Fine-tuning for each weapon */
        // bow
        if (weaponID == 2) {
          // Ctx.BulletSpeed = BulletSpeed - (BulletSpeed*0.08);
          // Ctx.BulletGravity = BulletGrav + (BulletGrav*0.05);
          bulletspeed = 10.08;
          bulletgrav = 10.05;
        }

        if (HeldID == -251) { // auto throw
          if (g_settings.no_nade_aim) {
            cancel_targeting();
            continue;
          }
          QAngle Angles_g = CalculateBestBoneAim(LPlayer, target, 999.9f);
          if (Angles_g.x == 0 && Angles_g.y == 0) {
            cancel_targeting();
            continue;
          }
          LPlayer.SetViewAngles(Angles_g);

        } else {
          QAngle Angles = CalculateBestBoneAim(LPlayer, target, max_fov);
          if (Angles.x == 0 && Angles.y == 0) {
            cancel_targeting();
            continue;
          }
          LPlayer.SetViewAngles(Angles);
        }
      }
    }
  }
  aim_t = false;
}
// Item Glow Stuff

static void item_glow_t() {
  item_t = true;
  while (item_t) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    while (g_Base != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(60));
      const auto g_settings = global_settings();
      if (!g_settings.item_glow) {
        break;
      }

      uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
      // item ENTs to loop, 10k-15k is normal. 10k might be better but will
      // not show all the death boxes i think.

      // for wish list
      std::vector<TreasureClue> new_treasure_clues;
      for (int i = 0; i < wish_list.size(); i++) {
        TreasureClue clue;
        clue.item_id = wish_list[i];
        clue.position = Vector(0, 0, 0);
        clue.distance = g_settings.aim_dist * 2;
        new_treasure_clues.push_back(clue);
      }

      for (int i = 0; i < itementcount; i++) {
        uint64_t centity = 0;
        apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
        if (centity == 0)
          continue;
        Item item = getItem(centity);

        // Item filter glow name setup and search.
        char glowName[200] = {0};
        uint64_t name_ptr;
        apex_mem.Read<uint64_t>(centity + OFFSET_MODELNAME, name_ptr);
        apex_mem.ReadArray<char>(name_ptr, glowName, 200);

        // item ids?
        uint64_t ItemID;
        apex_mem.Read<uint64_t>(centity + OFFSET_ITEM_ID, ItemID);
        /* uint64_t ItemID2;
        ItemID2 = ItemID % 301;
        printf("%ld\n", ItemID2); */
        // printf("Model Name: %s, Item ID: %lu\n", glowName, ItemID);
        // Level name printf
        // char LevelNAME[200] = { 0 };
        // uint64_t levelname_ptr;
        // apex_mem.Read<uint64_t>(g_Base + OFFSET_LEVELNAME, levelname_ptr);
        // apex_mem.ReadArray<char>(levelname_ptr, LevelNAME, 200);

        // printf("%s\n", LevelNAME);

        // Prints stuff you want to console
        // if (strstr(glowName, "mdl/"))
        //{
        // printf("%ld\n", ItemID);
        // }
        // Search model name and if true sets glow, must be a better way to do
        // this.. if only i got the item id to work..

        for (int i = 0; i < new_treasure_clues.size(); i++) {
          TreasureClue &clue = new_treasure_clues[i];
          if (ItemID == new_treasure_clues[i].item_id) {
            Vector position = item.getPosition();
            float distance = esp_local_pos.DistTo(position);
            if (distance < clue.distance) {
              clue.position = position;
              clue.distance = distance;
            }
            break;
          }
        }

        if (g_settings.loot.lightbackpack && ItemID == 207) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        /* else
        {
                std::array<unsigned char, 4> highlightFunctionBits = {
                        0,   // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
                        0,   // OutlineFunction HIGHLIGHT_OUTLINE_LOOT_SCANNED
                        64,
                        64
                };
                std::array<float, 3> highlightParameter = { 0, 0, 0 };
                apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS,
        0); static const int contextId = 0; int settingIndex = 99;
                apex_mem.Write<unsigned char>(centity +
        OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId, settingIndex); long
        highlightSettingsPtr; apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
        highlightSettingsPtr);
                apex_mem.Write<typeof(highlightFunctionBits)>(highlightSettingsPtr
        + 40 * settingIndex + 4, highlightFunctionBits);
                apex_mem.Write<typeof(highlightParameter)>(highlightSettingsPtr
        + 40 * settingIndex + 8, highlightParameter);
        } */
        if (g_settings.loot.medbackpack && ItemID == 208) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.heavybackpack && ItemID == 209) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.goldbackpack && ItemID == 210) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        // item id would help so much here, cant make them all the same color
        // so went with loba glow for body shield and helmet
        if (g_settings.loot.shieldupgrade1 &&
            (ItemID == 214748364993 || ItemID == 14073963583897798)) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);

          uint64_t ItemID;
          apex_mem.Read<uint64_t>(centity + OFFSET_ITEM_ID, ItemID);
          // uint64_t ItemID2;
          // ItemID2 = ItemID % 301;
          // printf("%ld\n", ItemID);
          // apex_mem.Write<typeof(highlightParameter)>(highlightSettingsPtr +
          // 40 * settingIndex + 8, highlightParameter);;
        }
        if (g_settings.loot.shieldupgrade2 &&
            (ItemID == 322122547394 || ItemID == 21110945375846599)) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgrade3 &&
            (ItemID == 429496729795 || ItemID == 52776987629977800)) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgrade4 && (ItemID == 429496729796)) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgrade5 && ItemID == 536870912201) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgradehead1 && ItemID == 188) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgradehead2 && ItemID == 189) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgradehead3 && ItemID == 190) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldupgradehead4 && ItemID == 191) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.accelerant && ItemID == 182) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.phoenix && ItemID == 183) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.skull &&
            strstr(glowName,
                   "mdl/Weapons/skull_grenade/skull_grenade_base_v.rmdl")) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (item.isBox()) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              0,   // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125, // OutlineFunction OutlineFunction
                   // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 88;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }

        if (item.isTrap()) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              0,   // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125, // OutlineFunction OutlineFunction
                   // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }

        // Gas Trap
        if (strstr(glowName,
                   "mdl/props/caustic_gas_tank/caustic_gas_tank.rmdl")) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              0,   // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125, // OutlineFunction OutlineFunction
                   // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.healthlarge && ItemID == 184) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.healthsmall && ItemID == 185) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldbattsmall && ItemID == 187) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shieldbattlarge && ItemID == 186) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.sniperammo && ItemID == 144) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.heavyammo && ItemID == 143) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.optic1xhcog && ItemID == 215) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lightammo && ItemID == 140) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.energyammo && ItemID == 141) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shotgunammo && ItemID == 142) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lasersight1 && ItemID == 229) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lasersight2 && ItemID == 230) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lasersight3 && ItemID == 231) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.sniperammomag1 && ItemID == 244) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.sniperammomag2 && ItemID == 245) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.sniperammomag3 && ItemID == 246) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.sniperammomag4 && ItemID == 247) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.energyammomag1 && ItemID == 240) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.energyammomag2 && ItemID == 241) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.energyammomag3 && ItemID == 242) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.energyammomag4 && ItemID == 243) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.stocksniper1 && ItemID == 255) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.stocksniper2 && ItemID == 256) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.stocksniper3 && ItemID == 257) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.stockregular1 && ItemID == 252) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.stockregular2 && ItemID == 253) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.stockregular3 && ItemID == 254) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shielddown1 && ItemID == 203) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shielddown2 && ItemID == 204) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shielddown3 && ItemID == 205) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shielddown4 && ItemID == 206) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lightammomag1 && ItemID == 232) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lightammomag2 && ItemID == 233) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lightammomag3 && ItemID == 234) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.lightammomag4 && ItemID == 235) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.heavyammomag1 && ItemID == 236) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.heavyammomag2 && ItemID == 237) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.heavyammomag3 && ItemID == 238) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.heavyammomag4 && ItemID == 239) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.optic2xhcog && ItemID == 216) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.opticholo1x && ItemID == 217) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.opticholo1x2x && ItemID == 218) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.opticthreat && ItemID == 219) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.optic3xhcog && ItemID == 220) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.optic2x4x && ItemID == 221) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.opticsniper6x && ItemID == 222) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.opticsniper4x8x && ItemID == 223) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.opticsniperthreat && ItemID == 224) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.suppressor1 && ItemID == 225) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.suppressor2 && ItemID == 226) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.suppressor3 && ItemID == 227) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.turbo_charger && ItemID == 258) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.skull_piecer && ItemID == 260) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.hammer_point && ItemID == 263) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.disruptor_rounds && ItemID == 262) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.boosted_loader && ItemID == 272) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shotgunbolt1 && ItemID == 248) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 72;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shotgunbolt2 && ItemID == 249) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shotgunbolt3 && ItemID == 250) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2941, 0, 0.5098};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 74;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.shotgunbolt4 && ItemID == 251) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.8431, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 75;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        // Nades
        if (g_settings.loot.grenade_frag && ItemID == 213) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }

        if (g_settings.loot.grenade_thermite && ItemID == 212) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.grenade_arc_star && ItemID == 214) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 70;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        // Weapons
        if (g_settings.loot.weapon_kraber && ItemID == 1) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_mastiff && ItemID == 3) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings.loot_filled, // InsideFunction
              125, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE
              64,  // OutlineRadius: size * 255 / 8
              64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess <<
                   // 7)
          };
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<int>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 2;
          int settingIndex = 67;
          apex_mem.Write<int>(centity + OFFSET_GLOW_ENABLE, contextId);
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          // apex_mem.Write<int>(ptr + OFFSET_HIGHLIGHTSERVERACTIVESTATES +
          // contextId, settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 0x28 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 0x28 * settingIndex + 8,
              highlightParameter);
        }
        if (g_settings.loot.weapon_lstar && ItemID == 7) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        // new gun, nemesis
        if (g_settings.loot.weapon_nemesis && ItemID == 135) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }

        if (g_settings.loot.weapon_havoc && ItemID == 13) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_devotion && ItemID == 18) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_triple_take && ItemID == 23) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_flatline && ItemID == 28) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_hemlock && ItemID == 33) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_g7_scout && ItemID == 39) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_alternator && ItemID == 44) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_r99 && ItemID == 49) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_prowler && ItemID == 56) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_volt && ItemID == 60) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0.2, 1, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 73;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_longbow && ItemID == 65) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_charge_rifle && ItemID == 70) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
          uint64_t ItemID;
          apex_mem.Read<uint64_t>(centity + OFFSET_ITEM_ID, ItemID);
          // printf("%ld\n", ItemID);
        }
        if (g_settings.loot.weapon_spitfire && ItemID == 75) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_r301 && ItemID == 80) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_eva8 && ItemID == 85) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_peacekeeper && ItemID == 90) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_mozambique && ItemID == 95) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_wingman && ItemID == 106) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_p2020 && ItemID == 111) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_re45 && ItemID == 116) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0.5490, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 66;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_sentinel && ItemID == 122) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 0, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 69;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_bow && ItemID == 127) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {1, 0, 0};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 67;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_3030_repeater && ItemID == 129) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_rampage && ItemID == 146) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }
        if (g_settings.loot.weapon_car_smg && ItemID == 151) {
          std::array<unsigned char, 4> highlightFunctionBits = {
              g_settings
                  .loot_filled, // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
              125,              // OutlineFunction OutlineFunction
                                // HIGHLIGHT_OUTLINE_LOOT_SCANNED
              64, 64};
          std::array<float, 3> highlightParameter = {0, 1, 1};
          apex_mem.Write<uint32_t>(centity + OFFSET_GLOW_THROUGH_WALLS, 2);
          static const int contextId = 0;
          int settingIndex = 65;
          apex_mem.Write<unsigned char>(
              centity + OFFSET_HIGHLIGHTSERVERACTIVESTATES + contextId,
              settingIndex);
          long highlightSettingsPtr;
          apex_mem.Read<long>(g_Base + HIGHLIGHT_SETTINGS,
                              highlightSettingsPtr);
          apex_mem.Write<typeof(highlightFunctionBits)>(
              highlightSettingsPtr + 40 * settingIndex + 4,
              highlightFunctionBits);
          apex_mem.Write<typeof(highlightParameter)>(
              highlightSettingsPtr + 40 * settingIndex + 8, highlightParameter);
        }

        // CREDITS to Rikkie
        // https://www.unknowncheats.me/forum/members/169606.html for all the
        // weapon ids and item ids code, you are a life saver!

      } // for(item) loop end
      treasure_clues = new_treasure_clues;
    } // while(item_glow) loop end
  }   // while(item_t) loop end
  item_t = false;
}

extern void start_overlay();

void terminal() {
  terminal_t = true;
  run_tui_menu();
}

int main(int argc, char *argv[]) {
  load_settings();

  if (geteuid() != 0) {
    // run as root..
    print_run_as_root();

    // test menu
    run_tui_menu();
    return 0;
  }

  const char *ap_proc = "r5apex.exe";

  std::thread aimbot_thr;
  std::thread esp_thr;
  std::thread actions_thr;
  std::thread cactions_thr;
  // Used to change things on a timer
  // std::thread updateInsideValue_thr;
  std::thread TriggerBotRun_thr;
  std::thread terminal_thr;
  std::thread overlay_thr;
  std::thread itemglow_thr;

  if (apex_mem.open_os() != 0) {
    exit(0);
  }

  while (active) {
    if (apex_mem.get_proc_status() != process_status::FOUND_READY) {
      if (aim_t) {
        aim_t = false;
        esp_t = false;
        actions_t = false;
        cactions_t = false;
        // Used to change things on a timer
        updateInsideValue_t = false;
        TriggerBotRun_t = false;
        terminal_t = false;
        overlay_t = false;
        item_t = false;
        g_Base = 0;
        quit_tui_menu();

        aimbot_thr.~thread();
        esp_thr.~thread();
        actions_thr.~thread();
        cactions_thr.~thread();
        // Used to change things on a timer
        // updateInsideValue_thr.~thread();
        TriggerBotRun_thr.~thread();
        terminal_thr.~thread();
        overlay_thr.~thread();
        itemglow_thr.~thread();
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
      printf("Searching for apex process...\n");

      apex_mem.open_proc(ap_proc);

      if (apex_mem.get_proc_status() == process_status::FOUND_READY) {
        g_Base = apex_mem.get_proc_baseaddr();
        printf("\nApex process found\n");
        printf("Base: %lx\n", g_Base);

        aimbot_thr = std::thread(AimbotLoop);
        esp_thr = std::thread(EspLoop);
        actions_thr = std::thread(DoActions);
        cactions_thr = std::thread(ClientActions);
        // Used to change things on a timer
        // updateInsideValue_thr = std::thread(updateInsideValue);
        TriggerBotRun_thr = std::thread(TriggerBotRun);
        itemglow_thr = std::thread(item_glow_t);
        aimbot_thr.detach();
        esp_thr.detach();
        actions_thr.detach();
        cactions_thr.detach();
        // Used to change things on a timer
        // updateInsideValue_thr.detach();
        TriggerBotRun_thr.detach();
        itemglow_thr.detach();
      }
    } else {
      apex_mem.check_proc();

      const auto g_settings = global_settings();
      const bool debug_mode = false;
      if (debug_mode) {
        if (terminal_t) {
          quit_tui_menu();
        }
      } else {
        if (!terminal_t) {
          terminal_thr = std::thread(terminal);
          terminal_thr.detach();
        }
        // wish_list.clear();
      }
      if (g_settings.no_overlay) {
        if (overlay_t) {
          overlay_t = false;
        }
      } else {
        if (!overlay_t) {
          overlay_thr = std::thread(start_overlay);
          overlay_thr.detach();
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return 0;
}
