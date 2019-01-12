// ////////////////////////////////////////////////////////////////////////////
// Forest region
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// Major Characters
// ////////////////////////////////////////////////////////////////////////////

object theGnoll
    article             "the "
    name                "gnoll"
    sex                 sexMale
    species             speciesGnoll
    faction             0
    rescued             0
;

object imp1
    article             "a "
    name                "tall male imp"
    sex                 sexMale
    species             speciesHuman
    faction             1
    $defeated           0
    ai function() {
        // 1-in-3 chance of performing a whirlwind attack
//        0 2 random do-whirl jump-false

//        *_0 $faction get-property random-not-faction
//        *_0 2 act-thrust $combat-node get-property call
//        end

//        LABEL do-whirl
//        *_0 1
//        act-swing $combat-node get-property call
    }
;

object imp2
    article             "a "
    name                "fat male imp"
    sex                 sexMale
    species             speciesHuman
    faction             1
//    skills map(
//        skl-melee 4;
//    );
    ai function() {
//        *_0 1 imp1 $ai getproperty call
    }
;

object imp3
    article             "a "
    name                "female imp"
    sex                 sexFemale
    species             speciesHuman
    faction             1
    ai function() {
//        *_0 1 imp-1 $ai get-property call
    }
;

object enemyEkeleton
    article             "a "
    name                "long-dead skeleton"
    sex                 sexNeuter
    species             speciesHuman
    faction             1
//    skills map(
//    );
    ai function() {
//        *_0 $faction get-property random-not-faction
//        *_0 2 act-thrust $combat-node get-property call
//        end
    }
;


// ////////////////////////////////////////////////////////////////////////////
// Locations
// ////////////////////////////////////////////////////////////////////////////

object forestPath
    location "On Forest Path"
    body function() {
        asm
        "You are on a path leading through a forest. The crumbling remains of a stone wall can be seen on one side, the words \"TOWN\" and a southward arrow has been painted on the wall. The wall partially hides an overgrown side trail." say
        "Go north into the forest" forestFork  add_option
        "Go south to town"         townGate    add_option
        "Take the overgrown trail" gnollCage   add_option
        "Rest Awhile"              restAwhile  add_option
    }
;

object townGate
    location "Outside the Town Gates"
    body function() {
        asm
        "You stand before a gate built into the large, wooden pallisade that surronds the town. The gate is open, ready to admit you to the market bzzaar within." say
        "Head into town" inTown add_option
        "Return to the forest" forestPath add_option
    }
;

object gnollCage
    location "By a Cage in the Forest"
    body function() {
        asm
        "You stand in a clearing in the forest. The area is dominated by a large, if rusty, cage." say
        "Return to the forest path" forestPath add_option

        $rescued theGnoll get_prop nognoll jnz
        " Inside the cage you can see a rather emaciated gnoll." say
        "Open the cage" openCage add_option
        "Try to talk to the gnoll" talkToCagedGnoll add_option
        return

        nognoll:
        " The cage stands empty and open." say
    }
;

object talkToCagedGnoll
    body function() {
        asm
        0 addContinue call

        $talkedToInCage theGnoll get_prop nottalked jz
        "The gnoll continues to plead for help in a barely audible voice. \"The imps...\"" say
        return

        nottalked:
        1 $talkedToInCage theGnoll set_prop
        "You can barely make out the gnoll's words; \"Help me, please! The imps, they have the key.\"" say
    }
;

object openCage
    body function() {
        asm
        0 addContinue call

        $defeated imp1 get_prop nokey jz
        // gnoll-cage-key item-qty no-key jump-false
        "Using the key you found you are able to open the cage; as you do, both the key and the lock turn to ash. As you open the door, the gnoll rushes out surprisingly fast given his state.\n\n\"Thank you!\" he says. \"Please, come see me in my home. I live to the north of the town.\"" say
        // gnoll-cage-key 1 remove-items
        1 $rescued theGnoll set_prop
        return

        nokey:
        "You rattle the door of the cage, but it is firmly locked." say
    }
;

object forestFork
    location "A Fork in the Path"
    body function() {
        asm
        "The forest path branches here. One fork leads northward, a faded sign labelling it as \"GNOLL HOME\". A northeast fork deeper into the forest, suitable for exploring what hides in the forest's shadowed reaches. To the south, the path leads back towards town. A western trail loop arounds to a discreet clearing hidden by dense brush." say

        "Take the north path"           outsideCabin       add_option
        "Explore the forest"            forestEvent        add_option
        "Check the discreet clearing"   discreetClearing   add_option
        "Return to the south"           forestPath         add_option
    }
;

object outsideCabin
    location "A Cabin in the Forest"
    body function() {
        asm
        "A small cabin has been built here, nestled between the trees." say

        $rescued theGnoll get_prop nognoll jnz
        " A trail of smoke can be seen curling up from the chimney; it appears someone is home!" say
        nognoll:
        "Re-eneter the forest" forestFork add_option
        "Try to enter the cabin." enteringCabin add_option
    }
;


// ////////////////////////////////////////////////////////////////////////////
// The Villanous Imps
// ////////////////////////////////////////////////////////////////////////////

object discreetClearing
    location "A Discreet Clearing"
    body function() {
        asm

        $defeated imp1 get_prop impsdone jnz
        "Three imps are gathered around a campfire, cackling and conversing. As you enter the clearing, they look up at you.\n\"An intruder! Get him!\"" say
        "\n[rng: " say 2 0 random stack_dup say "] " say
        fakewon jz

        "The sheer violence of the imps drives you back out of the clearing." say
        continueStr forestFork add_option
        return

        fakewon:
        mainImpFightAfter return
//        main-imp-fight-after reset-combat
//        imp-1 add-to-combat
//        imp-2 add-to-combat
//        imp-3 add-to-combat
//        return

        impsdone:
        "This is a quiet clearing screen from view from the path by the brush. The remains of a campfire are visible in the centre. No one else is here; the only thing you can do is leave." say
        "Leave the clearing" forestFork add_option
    }
;

object mainImpFightAfter
    body function() {
        asm
//        combat-status 0 player-defeat jump-lt

        // VICTORY
        1 $defeated imp1 set_prop
        "The imps lie unconscious around you, defeated. Scattered around the area, you find 200 gold and a large key.\n" say
//        gold 200 add-items
//        gnoll-cage-key 1 add-items
        0 addContinue call
        return

        // DEFEAT
//        LABEL player-defeat
//        >"You fall, defeated. You awaken some time later back in the forest.\n"
//        forest-fork add-continue
    }
;


// ////////////////////////////////////////////////////////////////////////////
// The Gnoll's Cabin
// ////////////////////////////////////////////////////////////////////////////

object enteringCabin
    body function() {
        asm
        $rescued theGnoll get_prop gnollhome jnz

        "Try as you might, the door is quite secure." say
        0 addContinue call
        return

        gnollhome:
        1 $visited theGnoll set_prop
        "You hear the voice of the gnoll call out from inside. \"Come on in!\"\n" say
        insideCabin
    }
;

object insideCabin
    location "Inside the Gnoll's Cabin"
    body function() {
        asm
        "The interior of the cabin is crampt, but cozy. You can see the gnoll slotched back on a wooden chair, a mug of some dark liquid in his hand.\n\n\"Good to see you again, my rescuer!\" he says as you enter." say
        "Talk to the gnoll" gnollCabinTalk     add_option
        "Leave"             outsideCabin       add_option
    }
;

object gnollCabinTalk
    body function() {
        asm
        "You have a nice chat with the gnoll." say
        0 addContinue call
    }
;


// ////////////////////////////////////////////////////////////////////////////
// Forest Random Events
// ////////////////////////////////////////////////////////////////////////////

object forestEvent
    events [
        forestEventSkeleton
        forestEventNothing
    ]
    body function() {
        asm $events self get_prop get_random
    }
;

object forestEventSkeleton
    //count { 1 }
    body function() {
        asm
        "You discover a menacing looking skeleton loitering around a grave. It stares at you with empty eye-sockets for a moment then charges with a soundless cry." say
        "\nThen vanishes as combat isn't implemented." say forestEventSkeletonAfter
//        forest-event-skeleton-after reset-combat
//        enemy-skeleton add-to-combat
    }
;
object forestEventSkeletonAfter
    body function() {
        asm
        0 addContinue call
//        combat-status 0 playerdefeat jump-lt

        // VICTORY
//        _5 0 20 random store
//        gold *_5 add-items
        "You have won the battle!" say
//        *_5 no-reward jump-false
//        >" You found " *_5 say-number >" gold pieces!"
//        LABEL no-reward
        "\n" say
        return

        playerdefeat:
        "You have been defeated and wake some time later alone in the forest.\n" say
    }
;

object forestEventNothing
    //count { 1 }
    body function() {
        asm
        "You stumble around in the forest for a while, but eventually end up back were you started without finding anything." say
        0 addContinue call
    }
;
