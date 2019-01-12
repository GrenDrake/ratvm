// ////////////////////////////////////////////////////////////////////////////
// Town region
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// Goods for Sale in Town
// ////////////////////////////////////////////////////////////////////////////

object teleportCrystal
    price 5
    article "a "
    name    "teleport crystal"
    plural  "teleport crystals"
    description "A crystal devised by Arnold of Va-Katr for the purpose of reaching the town quickly without the use of undue magic."
    onUse function() {
        asm
//        teleportCrystal 1 remove-items
//        "Light flashes and you vision goes blank. Moments later, the world returns."
//        say
//        town-gate
//        add-continue
    }
;

object gnollPotion
    price 25
    article "a "
    name    "potion of gnoll"
    plural  "potions of gnoll"
    description "A potion of transformation intended to make one more gnoll-like."
    onUse function() {
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

object humanPotion
    price 20
    article "a "
    name    "potion of humanity"
    plural  "potions of humanity"
    description "A potion of transformation intended to make one more human-like."
    onUse function() {
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

object commonSword
    price 20
    article "a "
    name    "common sword"
    plural  "common swords"
    description "A perfectly ordinary sword."
    slot    "weapon"
    canEquip function() {
        asm 1
    }
    actionList [
//        act-swing
//        act-thrust
    ]
;

object commonHelmet
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

object plateArmour
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

object inTown
    location "In Town"
    body function() {
        asm
        "You are inside the bazaar of the town; colourful stalls surround you and the sound of voices fills the air. The gate out of town is to the north while several stalls may be of interest. Particularly, there is a large tent with a sign reading \"Al the Alchemist Extraordinaire!\". Across the way from that is a strength-testing machine. Slightly further away, a blacksmith has set up their forge." say
        "Visit alchemist's tent"    townAlchemist          add_option
        "Visit the blacksmith"      townBlacksmith         add_option
        "Test your strength"        townStrengthTester     add_option
        "Leave town"                townGate               add_option
    }
;

function pronoun(who thePronoun) {
    asm
    thePronoun $pronouns $sex who get_prop get_prop get_item say
}

object townStrengthTester
    //  The purpose of this scene is to demonstrate making out-of-combat skill
    //  checks
    body function(: actor) {
        asm

        0 addContinue call

        thePlayer actor store // 0 party-at stk-dup say-uf
        $name actor get_prop say_uf
        " picks up the hammer sitting next to the strength testing machine and slams it down on the target as hard " say
        proSubject actor 2 pronoun call
        " can! " say
        -2 skillMelee thePlayer 3 skillCheck call
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

object townAlchemist
    //  Al the Alchemical Extraordinaire is an example of a very basic shop; Al
    //  offers no services beyond selling a few basic items.
    location "Al the Alchemist Extraordinaire's Tent"
    body function() {
        asm
        "You stand in an elaborately decorated tent with all manner of alchemical decor, such as bubbling flasks and vials of strangely coloured liquids, swatches of brightly coloured cloth, and cases of books with incomprehensible titles. An older man with an erect bearing beams at you from beside a counter." say
        "Buy something" townAlchemistTalk add_option
        "Leave the tent" inTown add_option
    }
;

object townAlchemistTalk
    saleList [
//        teleportCrystal
//        gnollPotion
//        humanPotion
    ]
    body function() {
        asm
        "\"Hello!\" he says. \"Would you like to buy something?\"\n" say

//        _9 townAlchemistTalk $saleList get_property list-size store
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

        "Buy nothing" $location world get_prop add_option
    }
;

object townAlchemistBuy
    body function() {
        asm
        continueStr townAlchemistTalk add_option
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

object townBlacksmith
    //  The Blacksmith is a more complicated shop that the alchemist and sells
    //  a variety of goods that the player will want. In addition, the player
    //  can engage the blacksmith in conversation on a variety of topics.
    location "The Town Blacksmith"
    body function() {
        asm
        "The blacksmith is located in a simple, but well built and maintained forge. The blacksmith, a large, heavily muscled human, works tirelessly." say
        "Buy something" townBlacksmithStore add_option
        "Talk to the blacksmith" townBlacksmithTalk add_option
        "Leave the tent" inTown add_option
    }
;

object townBlacksmithTalk
    body function() {
        asm
        "\"Welcome, " say $name $species thePlayer get_prop get_prop say "! How may I help you?\"" say
        "Ask about the town" townBlacksmithAboutTown add_option
        "Finish talking" $location world get_prop add_option
    }
;

object townBlacksmithAboutTown
    body function() {
        asm
        "You ask the blacksmith about the town.\n\"I admit, it's not large, but it's what you can find here that counts and you won't find any town with as many shops at this one anywhere in this world!\"" say
        continueStr townBlacksmithTalk add_option
    }
;

object townBlacksmithStore
    saleList [
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

        "Buy nothing" $location world get_prop add_option
    }
;

object townBlacksmithBuy
    body function() {
        asm
        continueStr townBlacksmithStore add_option
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
