/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license:
 * https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"

#include <cmath>
#include <limits>

namespace
{
    float g_currencyMultiplierRate = 1.0f;

    uint32 GetBoostedCount(uint32 count)
    {
        if (g_currencyMultiplierRate <= 1.0f || !count)
            return count;

        double product = static_cast<double>(count) * static_cast<double>(g_currencyMultiplierRate);

        if (!std::isfinite(product) ||
            product >= static_cast<double>(std::numeric_limits<uint32>::max()))
        {
            return std::numeric_limits<uint32>::max();
        }

        long long boosted = std::llround(product);

        if (boosted <= static_cast<long long>(count))
            return count;

        return static_cast<uint32>(boosted);
    }

    bool IsCurrencyItem(uint32 itemId)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        return proto && proto->IsCurrencyToken();
    }

    void AddCurrencyBonus(Player* player, uint32 itemId, uint32 count)
    {
        if (!player || !count || g_currencyMultiplierRate <= 1.0f || !IsCurrencyItem(itemId))
            return;

        uint32 boosted = GetBoostedCount(count);

        if (boosted > count)
            player->AddItem(itemId, boosted - count);
    }
}

class CurrencyMultiplierWorldScript : public WorldScript
{
public:
    CurrencyMultiplierWorldScript() : WorldScript("CurrencyMultiplierWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        g_currencyMultiplierRate = sConfigMgr->GetOption<float>("CurrencyMultiplier.Rate", 1.0f);

        if (!std::isfinite(g_currencyMultiplierRate) || g_currencyMultiplierRate < 1.0f)
        {
            LOG_ERROR("module.currencymultiplier",
                "CurrencyMultiplier.Rate must be a finite value >= 1; using 1.");
            g_currencyMultiplierRate = 1.0f;
        }
    }
};

class CurrencyMultiplierPlayerScript : public PlayerScript
{
public:
    CurrencyMultiplierPlayerScript() : PlayerScript("CurrencyMultiplierPlayerScript") { }

    void OnPlayerQuestRewardItem(Player* player, Item* item, uint32 count) override
    {
        if (item)
            AddCurrencyBonus(player, item->GetEntry(), count);
    }

    void OnPlayerCreateItem(Player* player, Item* item, uint32 count) override
    {
        if (item)
            AddCurrencyBonus(player, item->GetEntry(), count);
    }
};

class CurrencyMultiplierMiscScript : public MiscScript
{
public:
    CurrencyMultiplierMiscScript() : MiscScript("CurrencyMultiplierMiscScript") { }

    void OnAfterLootTemplateProcess(
        Loot* loot, LootTemplate const* /*tab*/, LootStore const& /*store*/, Player* /*lootOwner*/,
        bool /*personal*/, bool /*noEmptyError*/, uint16 /*lootMode*/) override
    {
        if (!loot || g_currencyMultiplierRate <= 1.0f)
            return;

        for (LootItem& item : loot->items)
            BoostLootItem(item);

        for (LootItem& item : loot->quest_items)
            BoostLootItem(item);
    }

private:
    static void BoostLootItem(LootItem& item)
    {
        if (IsCurrencyItem(item.itemid))
            item.count = GetBoostedCount(item.count);
    }
};

void AddCurrencyMultiplierScripts()
{
    new CurrencyMultiplierWorldScript();
    new CurrencyMultiplierPlayerScript();
    new CurrencyMultiplierMiscScript();
}
