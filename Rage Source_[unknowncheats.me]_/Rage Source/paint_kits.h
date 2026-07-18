#pragma once
#include <vector>
#include <string>
#include "imgui.h"

enum class SkinRarity {
    Consumer = 0,    // Gray
    Industrial,      // Light blue
    MilSpec,         // Blue
    Restricted,      // Purple
    Classified,      // Pink
    Covert,          // Red
    Contraband,      // Gold/Special
    COUNT
};

struct PaintKit {
    int id;
    const char* name;
    SkinRarity rarity;
};

inline ImVec4 GetRarityColor(SkinRarity rarity) {
    switch (rarity) {
        case SkinRarity::Consumer:   return ImVec4(0.69f, 0.75f, 0.78f, 1.0f); // #b0c3d9
        case SkinRarity::Industrial: return ImVec4(0.36f, 0.60f, 0.83f, 1.0f); // #5e98d9
        case SkinRarity::MilSpec:    return ImVec4(0.29f, 0.41f, 0.91f, 1.0f); // #4b69ff
        case SkinRarity::Restricted: return ImVec4(0.53f, 0.28f, 0.83f, 1.0f); // #8847ff
        case SkinRarity::Classified: return ImVec4(0.83f, 0.20f, 0.91f, 1.0f); // #d32ce6
        case SkinRarity::Covert:     return ImVec4(0.92f, 0.30f, 0.30f, 1.0f); // #eb4b4b
        case SkinRarity::Contraband: return ImVec4(0.89f, 0.68f, 0.22f, 1.0f); // #e4ae39
        default:                     return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

inline const std::vector<PaintKit> g_PaintKits = {

    // =============================================
    //  CONTRABAND / SPECIAL
    // =============================================
    { 344, "Dragon Lore", SkinRarity::Contraband },
    { 334, "Howl", SkinRarity::Contraband },
    { 44,  "Case Hardened", SkinRarity::Contraband },

    // =============================================
    //  COVERT — Rifles
    // =============================================
    { 675, "Bloodsport (AK)", SkinRarity::Covert },
    { 524, "Fuel Injector (AK)", SkinRarity::Covert },
    { 302, "Fire Serpent (AK)", SkinRarity::Covert },
    { 688, "Neon Revolution (AK)", SkinRarity::Covert },
    { 639, "Wild Lotus (AK)", SkinRarity::Covert },
    { 804, "Gold Arabesque (AK)", SkinRarity::Covert },
    { 955, "Nightwish (AK)", SkinRarity::Covert },
    { 1013, "Head Shot (AK)", SkinRarity::Covert },
    { 282, "Phantom Disruptor (AK)", SkinRarity::Covert },
    { 600, "The Empress (AK)", SkinRarity::Covert },
    { 474, "Aquamarine Revenge (AK)", SkinRarity::Covert },
    { 490, "Point Disarray (AK)", SkinRarity::Covert },
    { 981, "Ice Coaled (AK)", SkinRarity::Covert },

    { 309, "Howl (M4A4)", SkinRarity::Covert },
    { 400, "Dragon King (M4A4)", SkinRarity::Covert },
    { 255, "Asiimov (M4A4)", SkinRarity::Covert },
    { 844, "The Emperor (M4A4)", SkinRarity::Covert },
    { 1023, "Temukau (M4A4)", SkinRarity::Covert },
    { 533, "Desolate Space (M4A4)", SkinRarity::Covert },
    { 512, "Buzz Kill (M4A4)", SkinRarity::Covert },
    { 632, "Neo-Noir (M4A4)", SkinRarity::Covert },
    { 480, "Royal Paladin (M4A4)", SkinRarity::Covert },
    { 1092, "Cyber Security (M4A4)", SkinRarity::Covert },

    { 946, "Printstream (M4A1-S)", SkinRarity::Covert },
    { 681, "Mecha Industries (M4A1-S)", SkinRarity::Covert },
    { 548, "Chantico's Fire (M4A1-S)", SkinRarity::Covert },
    { 321, "Hyper Beast (M4A1-S)", SkinRarity::Covert },
    { 587, "Decimator (M4A1-S)", SkinRarity::Covert },
    { 631, "Leaded Glass (M4A1-S)", SkinRarity::Covert },
    { 1014, "Welcome to the Jungle (M4A1-S)", SkinRarity::Covert },
    { 1071, "Emphorosaur-S (M4A1-S)", SkinRarity::Covert },
    { 445, "Golden Coil (M4A1-S)", SkinRarity::Covert },
    { 440, "Icarus Fell (M4A1-S)", SkinRarity::Covert },
    { 360, "Cyrex (M4A1-S)", SkinRarity::Covert },
    { 430, "Knight (M4A1-S)", SkinRarity::Covert },
    { 695, "Player Two (M4A1-S)", SkinRarity::Covert },

    { 662, "Containment Breach (AWP)", SkinRarity::Covert },
    { 344, "Dragon Lore (AWP)", SkinRarity::Covert },
    { 279, "Asiimov (AWP)", SkinRarity::Covert },
    { 756, "Gungnir (AWP)", SkinRarity::Covert },
    { 803, "The Prince (AWP)", SkinRarity::Covert },
    { 475, "Hyper Beast (AWP)", SkinRarity::Covert },
    { 640, "Neo-Noir (AWP)", SkinRarity::Covert },
    { 451, "Medusa (AWP)", SkinRarity::Covert },
    { 662, "Wildfire (AWP)", SkinRarity::Covert },
    { 1025, "Chromatic Aberration (AWP)", SkinRarity::Covert },
    { 227, "Electric Hive (AWP)", SkinRarity::Covert },
    { 395, "Fever Dream (AWP)", SkinRarity::Covert },
    { 174, "Lightning Strike (AWP)", SkinRarity::Covert },
    { 424, "Man-o'-war (AWP)", SkinRarity::Covert },
    { 1101, "Doodle Lore (AWP)", SkinRarity::Covert },

    { 8,   "AUG | Akihabara Accept", SkinRarity::Covert },
    { 14,  "Akihabara Accept", SkinRarity::Covert },
    { 708, "AUG Momentum", SkinRarity::Covert },

    { 441, "SSG 08 | Dragonfire", SkinRarity::Covert },
    { 504, "SSG 08 | Bloodshot", SkinRarity::Covert },

    { 637, "SG 553 | Integrale", SkinRarity::Covert },

    { 523, "SCAR-20 | Bloodsport", SkinRarity::Covert },

    { 691, "FAMAS | Eye of Athena", SkinRarity::Covert },

    { 597, "Galil AR | Sugar Rush", SkinRarity::Covert },

    // =============================================
    //  COVERT — Pistols
    // =============================================
    { 351, "Deagle | Conspiracy", SkinRarity::Covert },
    { 527, "Deagle | Kumicho Dragon", SkinRarity::Covert },
    { 711, "Deagle | Mecha Industries", SkinRarity::Covert },
    { 816, "Deagle | Printstream", SkinRarity::Covert },
    { 901, "Deagle | Ocean Drive", SkinRarity::Covert },
    { 347, "Deagle | Blaze", SkinRarity::Covert },
    { 277, "Deagle | Hand Cannon", SkinRarity::Covert },
    { 37,  "Deagle | Golden Koi", SkinRarity::Covert },
    { 1084, "Deagle | Code Red", SkinRarity::Covert },

    { 506, "USP-S | Kill Confirmed", SkinRarity::Covert },
    { 653, "USP-S | Cortex", SkinRarity::Covert },
    { 313, "USP-S | Caiman", SkinRarity::Covert },
    { 318, "USP-S | Orion", SkinRarity::Covert },
    { 657, "USP-S | Neo-Noir", SkinRarity::Covert },
    { 814, "USP-S | Printstream", SkinRarity::Covert },
    { 1093, "USP-S | Ticket to Hell", SkinRarity::Covert },

    { 38,  "Glock | Fade", SkinRarity::Covert },
    { 623, "Glock | Bullet Queen", SkinRarity::Covert },
    { 680, "Glock | Wasteland Rebel", SkinRarity::Covert },
    { 694, "Glock | Neo-Noir", SkinRarity::Covert },
    { 586, "Glock | Water Elemental", SkinRarity::Covert },
    { 353, "Glock | Dragon Tattoo", SkinRarity::Covert },
    { 916, "Glock | Gamma Doppler", SkinRarity::Covert },
    { 1072, "Glock | Winterized", SkinRarity::Covert },

    { 696, "Five-SeveN | Angry Mob", SkinRarity::Covert },

    { 707, "P250 | Neon Rider", SkinRarity::Covert },

    { 664, "Tec-9 | Decimator", SkinRarity::Covert },

    { 650, "CZ75 | Xiangliu", SkinRarity::Covert },

    // =============================================
    //  COVERT — SMGs
    // =============================================
    { 507, "P90 | Asiimov", SkinRarity::Covert },
    { 565, "P90 | Shapewood", SkinRarity::Covert },

    { 690, "MAC-10 | Neon Rider", SkinRarity::Covert },
    { 1049, "MAC-10 | Sakkaku", SkinRarity::Covert },

    { 723, "MP9 | Hydra", SkinRarity::Covert },

    { 538, "MP7 | Bloodsport", SkinRarity::Covert },

    { 652, "UMP-45 | Crime Scene", SkinRarity::Covert },

    { 693, "PP-Bizon | Judgement of Anubis", SkinRarity::Covert },

    // =============================================
    //  COVERT — Shotguns & MGs
    // =============================================
    { 665, "XM1014 | Incinegator", SkinRarity::Covert },

    { 441, "MAG-7 | Justice", SkinRarity::Covert },

    { 626, "Nova | Wild Six", SkinRarity::Covert },

    { 689, "Sawed-Off | Devourer", SkinRarity::Covert },

    { 588, "Negev | Lionfish", SkinRarity::Covert },

    // =============================================
    //  COVERT — Knife Skins
    // =============================================
    { 38,  "Fade", SkinRarity::Covert },
    { 413, "Marble Fade", SkinRarity::Covert },
    { 568, "Emerald", SkinRarity::Covert },
    { 415, "Ruby (Doppler Phase)", SkinRarity::Covert },
    { 416, "Sapphire (Doppler Phase)", SkinRarity::Covert },
    { 417, "Black Pearl (Doppler Phase)", SkinRarity::Covert },
    { 418, "Doppler Phase 1", SkinRarity::Covert },
    { 419, "Doppler Phase 2", SkinRarity::Covert },
    { 420, "Doppler Phase 3", SkinRarity::Covert },
    { 421, "Doppler Phase 4", SkinRarity::Covert },
    { 569, "Lore", SkinRarity::Covert },
    { 573, "Autotronic", SkinRarity::Covert },
    { 570, "Gamma Doppler P1", SkinRarity::Covert },
    { 571, "Gamma Doppler P2", SkinRarity::Covert },
    { 572, "Gamma Doppler P3", SkinRarity::Covert },
    { 574, "Gamma Doppler P4", SkinRarity::Covert },
    { 409, "Tiger Tooth", SkinRarity::Covert },
    { 44,  "Case Hardened (Knife)", SkinRarity::Covert },
    { 12,  "Crimson Web", SkinRarity::Covert },
    { 59,  "Slaughter", SkinRarity::Covert },
    { 98,  "Ultraviolet", SkinRarity::Covert },
    { 558, "Damascus Steel", SkinRarity::Covert },
    { 410, "Rust Coat", SkinRarity::Covert },
    { 414, "Doppler (random)", SkinRarity::Covert },
    { 561, "Freehand", SkinRarity::Covert },
    { 580, "Bright Water (Knife)", SkinRarity::Covert },
    { 413, "Marble Fade (Knife)", SkinRarity::Covert },
    { 567, "Black Laminate", SkinRarity::Covert },
    { 43,  "Blue Steel", SkinRarity::Covert },
    { 735, "Vanilla / Default", SkinRarity::Covert },
    { 409, "Tiger Tooth (Knife)", SkinRarity::Covert },

    // =============================================
    //  CLASSIFIED
    // =============================================
    { 401, "Water Elemental", SkinRarity::Classified },
    { 645, "Corticera", SkinRarity::Classified },
    { 481, "Nemesis", SkinRarity::Classified },
    { 400, "Dragon King", SkinRarity::Classified },
    { 351, "Cartel", SkinRarity::Classified },
    { 1196, "Ice Coaled", SkinRarity::Classified },
    { 1144, "Printstream (Classified)", SkinRarity::Classified },
    { 289, "Redline", SkinRarity::Classified },
    { 254, "Tron", SkinRarity::Classified },
    { 453, "Neon Rider", SkinRarity::Classified },
    { 380, "Grand Prix", SkinRarity::Classified },
    { 180, "Chalice", SkinRarity::Classified },
    { 316, "Muertos", SkinRarity::Classified },
    { 230, "Guardian", SkinRarity::Classified },
    { 436, "Pit Viper", SkinRarity::Classified },
    { 497, "Frontside Misty", SkinRarity::Classified },
    { 422, "Torque", SkinRarity::Classified },

    // =============================================
    //  RESTRICTED
    // =============================================
    { 60,  "Dark Water", SkinRarity::Restricted },
    { 15,  "Blizzard Marbleized", SkinRarity::Restricted },
    { 371, "Griffin", SkinRarity::Restricted },
    { 1143, "Ticket to Hell", SkinRarity::Restricted },
    { 72,  "Safari Mesh", SkinRarity::Restricted },
    { 77,  "Bone Mask", SkinRarity::Restricted },
    { 33,  "Jungle DDPAT", SkinRarity::Restricted },
    { 111, "Blue Laminate", SkinRarity::Restricted },
    { 110, "Red Laminate", SkinRarity::Restricted },
    { 226, "Emerald Pinstripe", SkinRarity::Restricted },
    { 246, "Tuxedo", SkinRarity::Restricted },
    { 378, "Elite Build", SkinRarity::Restricted },

    // =============================================
    //  MIL-SPEC & BELOW
    // =============================================
    { 6,   "Arctic Camo", SkinRarity::MilSpec },
    { 396, "Urban Hazard", SkinRarity::MilSpec },
    { 316, "Abyss", SkinRarity::MilSpec },
    { 242, "Whiteout", SkinRarity::MilSpec },
    { 159, "Candy Apple", SkinRarity::MilSpec },
    { 3,   "Anodized Navy", SkinRarity::MilSpec },
    { 28,  "Contract", SkinRarity::MilSpec },
    { 32,  "Silver", SkinRarity::MilSpec },
    { 175, "Demolition", SkinRarity::MilSpec },
    { 5,   "Forest DDPAT", SkinRarity::Industrial },
    { 17,  "Tornado", SkinRarity::Industrial },
    { 21,  "Forest Leaves", SkinRarity::Industrial },
    { 25,  "Storm", SkinRarity::Industrial },
    { 2,   "Groundwater", SkinRarity::Consumer },
    { 1,   "Spray", SkinRarity::Consumer },
};
