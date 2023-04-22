$modelname "../grem.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -1 -90 
$cliptotextures

$bodygroup body 
{ 
studio "reference" 
} 

$attachment 0 "Bip01 R Hand" 25 0 0

$sequence "stand" "stand" fps 8 loop ACT_IDLE 1
$sequence "walk" "walk" LX fps 10 loop ACT_WALK 1 { event 1 1 }
$sequence "run" "run" LX fps 10 loop ACT_RUN 1 { event 1 1 }
$sequence "death" "death" fps 10 ACT_DIEFORWARD 1
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "spawn" "spawn" fps 3 ACT_HOVER 1 { event 8 5 }
$sequence "look" "look" fps 10 ACT_IDLE_ANGRY 1 { event 9 8 }
$sequence "jump" "jump" fps 10 ACT_LEAP 1 { event 2 3 } { event 3 9 }
$sequence "attk" "attk" fps 10 ACT_MELEE_ATTACK1 1 { event 11 6 }
$sequence "flip" "flip" fps 10 ACT_DIEBACKWARD 1 { event 5 7 }
$sequence "gfire" "gfire" fps 10 ACT_RANGE_ATTACK1 1 { event 12 1 }
$sequence "glook" "glook" fps 10 ACT_COMBAT_IDLE 1  { event 4 18 }
$sequence "gpain" "gpain" fps 10 ACT_SMALL_FLINCH 1
$sequence "grun" "grun" LX fps 10 loop ACT_RUN_SCARED 1
$sequence "lunge" "lunge" fps 10 ACT_MELEE_ATTACK2 1 { event 10 7 }
$sequence "maul" "maul" fps 10 loop ACT_SPECIAL_ATTACK1 1 { event 6 6 } { event 7 8 }
