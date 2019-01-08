// ////////////////////////////////////////////////////////////////////////////
// Town region
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// Goods for Sale in Town
// ////////////////////////////////////////////////////////////////////////////

object teleport-crystal
    price 5
    article "a "
    name    "teleport crystal"
    plural  "teleport crystals"
    description "A crystal devised by Arnold of Va-Katr for the purpose of reaching the town quickly without the use of undue magic."
    on-use function() {
        asm
//        teleport-crystal 1 remove-items
//        "Light flashes and you vision goes blank. Moments later, the world returns."
//        say
//        town-gate
//        add-continue
    }
;

object gnoll-potion
    price 25
    article "a "
    name    "potion of gnoll"
    plural  "potions of gnoll"
    description "A potion of transformation intended to make one more gnoll-like."
    on-use function() {
        asm
//        gnoll-potion 1 remove-items
//        add-return
//        the-player get-species
//        spc-gnoll already-gnoll jump-eq

//        the-player spc-gnoll set-species
//        >"You feel your body twist and change under the influence of the potion. Moments later, you have the body of a gnoll."
//        end

//        label already-gnoll
//        >"Nothing happens. Perhaps you can't become any more gnoll-like then you are?"
    }
;

object human-potion
    price 20
    article "a "
    name    "potion of humanity"
    plural  "potions of humanity"
    description "A potion of transformation intended to make one more human-like."
    on-use function() {
        asm
//        human-potion 1 remove-items
//        add-return
//        the-player get-species
//        spc-human already-human jump-eq

//        the-player spc-human set-species
//        >"You feel your body twist and change under the influence of the potion. Moments later, you have the body of a human."
//        end

//        label already-human
//        >"Nothing happens. Perhaps you can't become any more human-like then you are?"
    }
;

object common-sword
    price 20
    article "a "
    name    "common sword"
    plural  "common swords"
    description "A perfectly ordinary sword."
    slot    "weapon"
    can-equip function() {
        asm 1
    }
    action-list [
//        act-swing
//        act-thrust
    ]
;

object common-helmet
    price 15
    article "a "
    name    "common helmet"
    plural  "common helmets"
    slot    "head"
    description "A basic pot-helm."
    resistances {
//        dt-blunt 5;
//        dt-cutting 10;
    }
;

object plate-armour
    price 100
    article "a "
    name    "suit of plate armour"
    plural  "suits of plate armour"
    slot    "body"
    description "A suit of fine plate mail."
    resistances {
//        dt-blunt 20;
//        dt-cutting 35;
//        dt-piercing 15;
    }
    skills {
//        skl-dexterity -3;
    }
;


// ////////////////////////////////////////////////////////////////////////////
// Locations
// ////////////////////////////////////////////////////////////////////////////

object in-town
    location "In Town"
    body function() {
        asm
        "You are inside the bazaar of the town; colourful stalls surround you and the sound of voices fills the air. The gate out of town is to the north while several stalls may be of interest. Particularly, there is a large tent with a sign reading \"Al the Alchemist Extraordinaire!\". Across the way from that is a strength-testing machine. Slightly further away, a blacksmith has set up their forge." say
        "Visit alchemist's tent"    town-alchemist          add-option
        "Visit the blacksmith"      town-blacksmith         add-option
        "Test your strength"        town-strength-tester    add-option
        "Leave town"                town-gate               add-option
    }
;

function pronoun(who the-pronoun) {
    asm
    the-pronoun $pronouns $sex who get-prop get-prop get-item say
}

object town-strength-tester
    //  The purpose of this scene is to demonstrate making out-of-combat skill
    //  checks
    body function(: actor) {
        asm

        0 add-continue call

        player actor store // 0 party-at stk-dup say-uf
        $name actor get-prop say-uf
        " picks up the hammer sitting next to the strength testing machine and slams it down on the target as hard " say
        pro-subject actor 2 pronoun call
        " can! " say
        -2 skill-melee player 3 skill-check call
        failed jz

        "\nThe indicator shoots up, hitting the bell and making it ring loudly!" say
        return
        failed:
        "\nThe indicator shoots up, but falls again before it reaches the bell." say
    }
;

// ////////////////////////////////////////////////////////////////////////////
// The Alchemist
// ////////////////////////////////////////////////////////////////////////////

object town-alchemist
    //  Al the Alchemical Extraordinaire is an example of a very basic shop; Al
    //  offers no services beyond selling a few basic items.
    location "Al the Alchemist Extraordinaire's Tent"
    body function() {
        asm
        "You stand in an elaborately decorated tent with all manner of alchemical decor, such as bubbling flasks and vials of strangely coloured liquids, swatches of brightly coloured cloth, and cases of books with incomprehensible titles. An older man with an erect bearing beams at you from beside a counter." say
        "Buy something" town-alchemist-talk add-option
        "Leave the tent" in-town add-option
    }
;

object town-alchemist-talk
    sale-list [
//        teleport-crystal
//        gnoll-potion
//        human-potion
    ]
    body function() {
        asm
        "\"Hello!\" he says. \"Would you like to buy something?\"\n" say

//        _9 town-alchemist-talk $sale-list get-property list-size store
                        // the number of items for sale
//        _8 0 store      // current list position

//        LABEL list-start
//        *_8 *_9 list-done jump-gte

//        _7 town-alchemist-talk $sale-list get-property *_8 list-get store
//        *_7 say-uf
//        >" is "
//        *_7 $price get-property say-number
//        >" gold pieces\n"

//        gold item-qty *_7 $price get-property cant-afford jump-lt
//        *_7 $name get-property town-alchemist-buy *_7 add-option-xtra
//        LABEL cant-afford

        // go to next item
//        _8 *_8 increment store
//        list-start jump

//        LABEL list-done

        "Buy nothing" $location world get-prop add-option
    }
;

object town-alchemist-buy
    body function() {
        asm
        continue-str town-alchemist-talk add-option
//        gold *_0 $price get-property remove-items
//        cant-afford jump-false

//        *_0 1 add-items
//        >"You buy "
//        *_0 say
//        >"."
//        end

//        LABEL cant-afford
//        >"You can't afford that!"
    }
;

// ////////////////////////////////////////////////////////////////////////////
// The Blacksmith
// ////////////////////////////////////////////////////////////////////////////

object town-blacksmith
    //  The Blacksmith is a more complicated shop that the alchemist and sells
    //  a variety of goods that the player will want. In addition, the player
    //  can engage the blacksmith in conversation on a variety of topics.
    location "The Town Blacksmith"
    body function() {
        asm
        "The blacksmith is located in a simple, but well built and maintained forge. The blacksmith, a large, heavily muscled human, works tirelessly." say
        "Buy something" town-blacksmith-store add-option
        "Talk to the blacksmith" town-blacksmith-talk add-option
        "Leave the tent" in-town add-option
    }
;

object town-blacksmith-talk
    body function() {
        asm
        "\"Welcome, " say $name $species player get-prop get-prop say "! How may I help you?\"" say
        "Ask about the town" town-blacksmith-abouttown add-option
        "Finish talking" $location world get-prop add-option
    }
;

object town-blacksmith-abouttown
    body function() {
        asm
        "You ask the blacksmith about the town.\n\"I admit, it's not large, but it's what you can find here that counts and you won't find any town with as many shops at this one anywhere in this world!\"" say
        continue-str town-blacksmith-talk add-option
    }
;

object town-blacksmith-store
    sale-list [
//        common-sword
//        common-helmet
//        plate-armour
    ]
    body function() {
        asm
        "\"What would you like?\"\n" say

//        _9 town-blacksmith-store $sale-list get-property list-size store
                        // the number of items for sale
//        _8 0 store      // current list position

//        LABEL list-start
//        *_8 *_9 list-done jump-gte

//        _7 town-blacksmith-store $sale-list get-property *_8 list-get store
//        *_7 say-uf
//        >" is "
//        *_7 $price get-property say-number
//        >" gold pieces\n"

//        gold item-qty *_7 $price get-property cant-afford jump-lt
//        *_7 $name get-property town-blacksmith-buy *_7 add-option-xtra
//        LABEL cant-afford

        // go to next item
//        _8 *_8 increment store
//        list-start jump

//        LABEL list-done

        "Buy nothing" $location world get-prop add-option
    }
;

object town-blacksmith-buy
    body function() {
        asm
        continue-str town-blacksmith-store add-option
//        gold *_0 $price get-property remove-items
//        cant-afford jump-false

//        *_0 1 add-items
//        >"You buy "
//        *_0 say
//        >"."
//        end

//        LABEL cant-afford
//        >"You can't afford that!"
    }
;
