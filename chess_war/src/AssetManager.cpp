#include "AssetManager.h"
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// ============================================================================
// 聲學合成輔助函式 (Synthesized Audio Helpers)
// ============================================================================

static Sound synth_sine_beep(float freq, float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        samples[i] = (short)(32767.0f * sinf(2.0f * PI * freq * t) * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static Sound synth_sweep(float freqStart, float freqEnd, float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        float phase = 2.0f * PI * (freqStart * t + 0.5f * (freqEnd - freqStart) * t * t / duration);
        samples[i] = (short)(32767.0f * sinf(phase) * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static Sound synth_noise(float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        float randomValue = (float)GetRandomValue(-32767, 32767);
        samples[i] = (short)(randomValue * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static Sound synth_chord(float freq1, float freq2, float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        
        float s1 = sinf(2.0f * PI * freq1 * t);
        float s2 = sinf(2.0f * PI * freq2 * t);
        samples[i] = (short)(32767.0f * (s1 + s2) * 0.5f * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static Sound synth_arpeggio(float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    float freqs[4] = { 261.63f, 329.63f, 392.00f, 523.25f };
    float note_dur = duration / 4.0f;
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        int note_idx = (int)(t / note_dur);
        if (note_idx > 3) note_idx = 3;
        
        float note_t = t - (note_idx * note_dur);
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        
        samples[i] = (short)(32767.0f * sinf(2.0f * PI * freqs[note_idx] * note_t) * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static Sound synth_victory(float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    float freqs[4] = { 261.63f, 329.63f, 392.00f, 523.25f };
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        
        float s = 0.0f;
        for (int f = 0; f < 4; ++f) {
            s += sinf(2.0f * PI * freqs[f] * t);
        }
        samples[i] = (short)(32767.0f * (s / 4.0f) * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static Sound synth_defeat(float duration, float volume) {
    int sampleRate = 44100;
    int frameCount = (int)(sampleRate * duration);
    short *samples = (short *)MemAlloc(frameCount * sizeof(short));
    
    float freqs[4] = { 220.00f, 261.63f, 329.63f, 440.00f };
    
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / duration);
        if (envelope < 0.0f) envelope = 0.0f;
        
        float bend = 1.0f - 0.2f * (t / duration);
        
        float s = 0.0f;
        for (int f = 0; f < 4; ++f) {
            s += sinf(2.0f * PI * freqs[f] * bend * t);
        }
        samples[i] = (short)(32767.0f * (s / 4.0f) * envelope * volume);
    }
    
    Wave wave = { 0 };
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;
    
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

// ============================================================================
// AssetManager 實作
// ============================================================================

// 取得單例實例
AssetManager& AssetManager::get_instance() {
    static AssetManager instance;
    return instance;
}

// 建構子：初始化色彩表 (Sleek Dark Mode 色彩調和)
AssetManager::AssetManager() {
    m_color_bg             = Color{ 15, 23, 42, 255 };      // 深灰藍 (#0f172a)
    m_color_panel          = Color{ 30, 41, 59, 255 };      // 面板灰藍 (#1e293b)
    m_color_grid_light     = Color{ 45, 55, 72, 255 };      // 棋盤亮色格 (Slate 700 / HSL 較淺)
    m_color_grid_dark      = Color{ 26, 32, 44, 255 };      // 棋盤暗色格 (Slate 900 / HSL 較深)
    m_color_hero_a         = Color{ 139, 92, 246, 255 };    // 英雄 A 紫色 (Violet 500)
    m_color_hero_b         = Color{ 6, 182, 212, 255 };     // 英雄 B 青色 (Cyan 500)
    m_color_enemy          = Color{ 244, 63, 94, 255 };     // 敵方怪物玫紅 (Rose 500)
    m_color_text_primary   = Color{ 248, 250, 252, 255 };   // 主文字白 (Slate 50)
    m_color_text_secondary = Color{ 203, 213, 225, 255 };   // 次文字灰 (Slate 300)
    m_color_accent_gold    = Color{ 245, 158, 11, 255 };    // 強調金黃 (Amber 500)
    m_color_hp             = Color{ 239, 68, 68, 255 };     // 生命紅 (Red 500)
    m_color_ap             = Color{ 16, 185, 129, 255 };    // 行動點綠 (Emerald 500)
    m_color_xp             = Color{ 59, 130, 246, 255 };    // 經驗值藍 (Blue 500)
    
    // 初始化字型與音效為預設空值
    m_font = { 0 };
    m_sound_click = { 0 };
    m_sound_move = { 0 };
    m_sound_attack = { 0 };
    m_sound_hit = { 0 };
    m_sound_death = { 0 };
    m_sound_promotion = { 0 };
    m_sound_turn_player = { 0 };
    m_sound_turn_enemy = { 0 };
    m_sound_victory = { 0 };
    m_sound_defeat = { 0 };
    m_language = Language::ZH;
}

void AssetManager::initialize() {
    // 1. 初始化音訊裝置
    InitAudioDevice();
    if (IsAudioDeviceReady()) {
        TraceLog(LOG_INFO, "AUDIO: Audio device initialized successfully.");
        
        // 2. 在記憶體中動態合成 interactive 8-bit 音效
        m_sound_click        = synth_sine_beep(600.0f, 0.05f, 0.4f);
        m_sound_move         = synth_sweep(350.0f, 550.0f, 0.12f, 0.35f);
        m_sound_attack       = synth_sweep(650.0f, 350.0f, 0.15f, 0.4f);
        m_sound_hit          = synth_noise(0.08f, 0.35f);
        m_sound_death        = synth_noise(0.25f, 0.45f);
        m_sound_promotion    = synth_arpeggio(0.4f, 0.45f);
        m_sound_turn_player  = synth_chord(440.0f, 554.37f, 0.35f, 0.35f); // A4 + C#5 明亮大三度
        m_sound_turn_enemy   = synth_chord(220.0f, 261.63f, 0.35f, 0.35f); // A3 + C4 低沉小三度
        m_sound_victory      = synth_victory(1.2f, 0.5f);
        m_sound_defeat       = synth_defeat(1.5f, 0.5f);
    } else {
        TraceLog(LOG_WARNING, "AUDIO: Failed to initialize audio device.");
    }

    // 3. 設定字型支援繁體中文的 Unicode 字集
    std::string chinese_chars = 
        "棋境戰爭遊戲狀態回合：階段：玩家回合敵方回合英雄 A (紫)英雄 B (青)血量：行動點：經驗值：當前身份：抽牌堆：棄牌堆：解鎖規則：兵後退馬越步象穿透無手牌列表：消耗：1 AP點擊綠色格子進行 [移動]點擊紅色格子進行 [攻擊]SPACE: 結束回合控制：滑鼠右鍵取消 | ESC 鍵退出英雄升變！選擇以下三個升級之一來強化您的英雄：+1 攻擊力增加英雄的\n基礎傷害 1 點。+1 最大生命值增加最大生命值 1 點\n並治療 1 點生命值。士兵後退士兵卡牌現在可以\n後退移動與攻擊。騎士躍擊騎士卡牌現在可以\n進行 3x1 距離跳躍。主教穿透主教攻擊現在可以\n穿透棋盤障礙物。選擇戰役勝利全軍覆沒累計擊殺怪物數量：您已成功捍衛了西洋棋王國！兩位英雄皆已陣亡。西洋棋王國瓦解了...按下 [R] 鍵重新開始 | [ESC] 鍵退出擊殺進度：玩家回合敵方回合行動點重置！怪物行動中...底線升變！已擊殺！選擇：以士兵方式移動/攻擊選擇：以騎士方式移動/攻擊選擇：以主教方式移動/攻擊選擇：以城堡方式移動/攻擊選擇：以國王方式移動/攻擊選擇：以皇后方式移動/攻擊選擇：以臨時皇后方式移動/攻擊士兵騎士主教城堡國王皇后臨時皇后兵馬象車后王無狀態 +1 AP -1 HPKILLED!PROMOTED!Unknown城堡卡"
        "English中文ENZH請選擇返回主選單再玩一局";

    // 4. 動態收集獨一無二的碼點 (ASCII 32-126 + L10n + 中文碼點)
    int rawCount = 0;
    int *rawCodepoints = LoadCodepoints(chinese_chars.c_str(), &rawCount);
    
    std::set<int> uniqueSet;
    for (int i = 32; i < 127; ++i) {
        uniqueSet.insert(i);
    }
    for (int i = 0; i < rawCount; ++i) {
        uniqueSet.insert(rawCodepoints[i]);
    }
    UnloadCodepoints(rawCodepoints);
    for (const auto& entry : L10n::table()) {
        int enCount = 0;
        int* enCps = LoadCodepoints(entry.second.first.c_str(), &enCount);
        for (int i = 0; i < enCount; ++i) {
            uniqueSet.insert(enCps[i]);
        }
        UnloadCodepoints(enCps);
        int zhCount = 0;
        int* zhCps = LoadCodepoints(entry.second.second.c_str(), &zhCount);
        for (int i = 0; i < zhCount; ++i) {
            uniqueSet.insert(zhCps[i]);
        }
        UnloadCodepoints(zhCps);
    }
    
    int cpCount = (int)uniqueSet.size();
    std::vector<int> codepoints(uniqueSet.begin(), uniqueSet.end());

    constexpr int kFontPx = 80;
    std::vector<std::string> candidates;
    auto tryAdd = [&](const std::string& path) {
        if (!path.empty() && FileExists(path.c_str())) {
            candidates.push_back(path);
        }
    };
    const std::string exeDir = GetApplicationDirectory();
    const char* taipeiNames[] = {"TaipeiSansTCBeta-Regular.ttf", "TaipeiSansTC-Regular.ttf",
                                 nullptr};
    for (const char** name = taipeiNames; *name != nullptr; ++name) {
        tryAdd("assets/fonts/" + std::string(*name));
        tryAdd(exeDir + "assets/fonts/" + std::string(*name));
    }
    tryAdd("C:/Windows/Fonts/TaipeiSansTCBeta-Regular.ttf");
    tryAdd("C:/Windows/Fonts/TaipeiSansTC-Regular.ttf");
    tryAdd("C:/Windows/Fonts/NotoSansTC-Regular.otf");
    tryAdd("C:/Windows/Fonts/NotoSansTC-Medium.otf");
    tryAdd("assets/fonts/kaiu.ttf");
    tryAdd(exeDir + "assets/fonts/kaiu.ttf");
    tryAdd("C:/Windows/Fonts/kaiu.ttf");
    tryAdd("assets/fonts/msjh.ttc");
    tryAdd(exeDir + "assets/fonts/msjh.ttc");

    for (const std::string& path : candidates) {
        Font candidate = LoadFontEx(path.c_str(), kFontPx, codepoints.data(), cpCount);
        if (candidate.texture.id != 0) {
            m_font = candidate;
            SetTextureFilter(m_font.texture, TEXTURE_FILTER_BILINEAR);
            TraceLog(LOG_INFO, "FONT: loaded from %s", path.c_str());
            return;
        }
    }
    TraceLog(LOG_WARNING, "FONT: fallback default; run bomberman/scripts/fetch_ui_font.py");
    m_font = GetFontDefault();
}

void AssetManager::cleanup() {
    // 1. 卸載字型資源
    if (m_font.texture.id != 0) {
        UnloadFont(m_font);
        TraceLog(LOG_INFO, "FONT: Unloaded font resources.");
    }

    // 2. 卸載音效資源
    if (IsAudioDeviceReady()) {
        UnloadSound(m_sound_click);
        UnloadSound(m_sound_move);
        UnloadSound(m_sound_attack);
        UnloadSound(m_sound_hit);
        UnloadSound(m_sound_death);
        UnloadSound(m_sound_promotion);
        UnloadSound(m_sound_turn_player);
        UnloadSound(m_sound_turn_enemy);
        UnloadSound(m_sound_victory);
        UnloadSound(m_sound_defeat);
        TraceLog(LOG_INFO, "AUDIO: Unloaded all synthesized sound effects.");
        
        CloseAudioDevice();
        TraceLog(LOG_INFO, "AUDIO: Closed audio device.");
    }
}
