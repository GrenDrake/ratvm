declare TITLE   "Automated Test Suite for Vocab and the Dictionary";
declare AUTHOR  "Gren Drake";
declare VERSION 1;
declare GAMEID  "";

declare vocabInput  [ "joy" "to" "the" "world" ];
declare vocabResult [ `joy` `to` `the` `world` ];

function main() {
    [ i v ]

    // test checking if index is valid vocab
    (if (not (is_valid `the`)) (error "Reported valid vocab entry as invalid."))
    (if (is_valid (astype 5 Vocab)) (error "Reported invalid vocab entry as valid (5)."))
    (if (is_valid (astype 500 Vocab)) (error "Reported invalid vocab entry as valid (500)."))

    // test whether dictionary lookups return right result
    (set i 0)
    (while (lt i (size vocabInput))
        (proc
            (set v (get_dict (get vocabInput i)))
            (if (neq v (get vocabResult i)) (error "dictionary lookup failed."))
            (inc i)))

    // test creating string with dictionary word
    (if (str_compare (string `the`) "the") (error "conversion of dictionary word to string failed"))
}
