// * THESE BUTTON COLLECTIONS MUST BE BASE TEMPLATES WITHOUT UPGRADES.

#include "Buttons.h"

#include "Util.h"

#define BUILD_AGE_1                                              \
    { ICONTYPE_BUILD, { ICONBUILD_BARRACKS    }, TRIGGER_NONE }, \
    { ICONTYPE_BUILD, { ICONBUILD_MILL        }, TRIGGER_NONE }, \
    { ICONTYPE_BUILD, { ICONBUILD_HOUSE       }, TRIGGER_NONE }, \
    { ICONTYPE_BUILD, { ICONBUILD_OUTPOST     }, TRIGGER_NONE }, \
    { ICONTYPE_BUILD, { ICONBUILD_STONE_CAMP  }, TRIGGER_NONE }, \
    { ICONTYPE_BUILD, { ICONBUILD_LUMBER_CAMP }, TRIGGER_NONE }

#define BUILD_AGE_3                                              \
    { ICONTYPE_BUILD, { ICONBUILD_TOWN_CENTER }, TRIGGER_NONE }, \
    { ICONTYPE_BUILD, { ICONBUILD_CASTLE      }, TRIGGER_NONE }

#define BARRACKS_AGE_1 \
    { ICONTYPE_UNIT, { ICONUNIT_MILITIA }, TRIGGER_NONE }

#define BARRACKS_AGE_2 \
    { ICONTYPE_TECH, { ICONTECH_RESEARCH_MAN_AT_ARMS }, TRIGGER_UPGRADE_MILITIA }

#define TOWN_CENTER_AGE_1                                              \
    { ICONTYPE_UNIT, { ICONUNIT_MALE_VILLAGER   }, TRIGGER_NONE     }, \
    { ICONTYPE_UNIT, { ICONUNIT_FEMALE_VILLAGER }, TRIGGER_NONE     }, \
    { ICONTYPE_TECH, { ICONTECH_AGE_2           }, TRIGGER_AGE_UP_2 }

static const Button build_age1[] = { BUILD_AGE_1 };
static const Button build_age2[] = { BUILD_AGE_1 };
static const Button build_age3[] = { BUILD_AGE_1, BUILD_AGE_3 };
static const Button build_age4[] = { BUILD_AGE_1, BUILD_AGE_3 };

static const Button* GetBuilding(const Age age)
{
    const Button* ages[] = {
        build_age1,
        build_age2,
        build_age3,
        build_age4,
    };
    return ages[age];
}

static int32_t GetBuildingLen(const Age age)
{
    const int32_t lens[] = {
        UTIL_LEN(build_age1),
        UTIL_LEN(build_age2),
        UTIL_LEN(build_age3),
        UTIL_LEN(build_age4),
    };
    return lens[age];
}

static const Button barracks_age1[] = { BARRACKS_AGE_1 };
static const Button barracks_age2[] = { BARRACKS_AGE_1, BARRACKS_AGE_2 };
static const Button barracks_age3[] = { BARRACKS_AGE_1, BARRACKS_AGE_2 };
static const Button barracks_age4[] = { BARRACKS_AGE_1, BARRACKS_AGE_2 };

static const Button* GetBarracks(const Age age)
{
    const Button* ages[] = {
        barracks_age1,
        barracks_age2,
        barracks_age3,
        barracks_age4,
    };
    return ages[age];
}

static int32_t GetBarracksLen(const Age age)
{
    const int32_t lens[] = {
        UTIL_LEN(barracks_age1),
        UTIL_LEN(barracks_age2),
        UTIL_LEN(barracks_age3),
        UTIL_LEN(barracks_age4),
    };
    return lens[age];
}

static const Button towncenter_age1[] = { TOWN_CENTER_AGE_1 };
static const Button towncenter_age2[] = { TOWN_CENTER_AGE_1 };
static const Button towncenter_age3[] = { TOWN_CENTER_AGE_1 };
static const Button towncenter_age4[] = { TOWN_CENTER_AGE_1 };

static const Button* GetTownCenter(const Age age)
{
    const Button* ages[] = {
        towncenter_age1,
        towncenter_age2,
        towncenter_age3,
        towncenter_age4,
    };
    return ages[age];
}

static int32_t GetTownCenterLen(const Age age)
{
    const int32_t lens[] = {
        UTIL_LEN(towncenter_age1),
        UTIL_LEN(towncenter_age2),
        UTIL_LEN(towncenter_age3),
        UTIL_LEN(towncenter_age4),
    };
    return lens[age];
}

Buttons Buttons_FromMotive(const Motive motive, const Age age)
{
    static Buttons zero;
    Buttons buttons = zero;
    switch(motive.action)
    {
    default:
        break;
    case ACTION_BUILD:
        buttons.button = GetBuilding(age);
        buttons.count = GetBuildingLen(age);
        break;
    case ACTION_UNIT_TECH:
        switch(motive.type)
        {
        case TYPE_BARRACKS:
            buttons.button = GetBarracks(age);
            buttons.count = GetBarracksLen(age);
            break;
        case TYPE_TOWN_CENTER:
            buttons.button = GetTownCenter(age);
            buttons.count = GetTownCenterLen(age);
            break;
        default:
            break;
        }
        break;
    }
    return buttons;
}

bool Buttons_IsIndexValid(const Buttons buttons, const int32_t index)
{
    return index != -1 && buttons.button != NULL && index < buttons.count;
}
