$modelname "hknight.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -18 -90 

$bodygroup body 
{ 
studio "reference" 
}

$sequence "stand" "stand" loop fps 2 ACT_IDLE 1
$sequence "walk" "walk" LX loop fps 10 ACT_WALK 1 { event 3 1 }
$sequence "run" "run" LX loop fps 10 ACT_RUN 1 { event 3 1 }
$sequence "slice" "slice" LX loop fps 10 ACT_MELEE_ATTACK1 1 { event 5 5 } { event 5 6 } { event 5 7 } { event 5 8 } 
$sequence "smash" "smash" LX loop fps 10 ACT_MELEE_ATTACK1 1 { event 5 5 } { event 5 6 } { event 5 7 } { event 5 8 } { event 5 9 } 
$sequence "w_attack" "w_attack" LX loop fps 10 ACT_MELEE_ATTACK1 1 { event 5 4 } { event 5 5 } { event 5 6 } { event 5 10 } { event 5 11 } { event 5 12 } { event 5 17 } { event 5 18 } { event 5 19 }
$sequence "char_a" "char_a" LX loop fps 10 ACT_SPECIAL_ATTACK1 1 { event 5 5 } { event 5 6 } { event 5 7 } { event 5 8 } { event 5 9 } { event 5 10 }
$sequence "char_b" "char_b" LX loop fps 10
$sequence "magica" "magica" fps 10
$sequence "magicb" "magicb" fps 10
$sequence "magicc" "magicc" fps 10 ACT_RANGE_ATTACK1 1 { event 4 6 "-2" } { event 4 7 "-1" } { event 4 8 "0" } { event 4 9 "1" } { event 4 10 "2" } { event 4 11 "3" }
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1
$sequence "deathb" "deathb" fps 10 ACT_DIESIMPLE 1
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
