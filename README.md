# mod-currency-multiplier

Multiplies currency-token items gained from:

- loot: boss drops, gameobject caches, item containers/satchels, master loot,
  group roll and auto-looted currency;
- quest rewards;
- items created by spells.

It uses the core's currency-token set (`ItemTemplate::IsCurrencyToken()`),
which includes Emblems, Stone Keeper's Shards, badges, battleground marks,
Champion's Seal, Dalaran tokens and similar items.

## Configuration

Edit `mod-currency-multiplier.conf` in `etc/modules/`:

```ini
CurrencyMultiplier.Rate = 3
```

- `1` disables the module.
- Values must be finite and `>= 1`; invalid values fall back to `1`.
- Loot counts are boosted before the loot is distributed, so the loot window
  already shows the boosted amount.
- Quest rewards and spell-created items get the extra amount as a second
  "receive item" message.

## Not affected

The module deliberately does not hook vendor purchases, refunds, trades, mail
attachments, bank, auction or buyback. That avoids currency exchange and refund
duplication.

Currency from SAI, scripts, GM `.additem` and mail-delivered rewards is not
boosted. Honor and arena points use the core `Rate.Honor` and
`Rate.ArenaPoints`; looted gold uses `Rate.Drop.Money`.

Older tokens that are not marked as currency tokens, such as Apexis Shards or
Halaa tokens, are not affected.

## Installation

Place the module in `modules/mod-currency-multiplier`, rebuild the worldserver,
and restart. No core files are modified.
