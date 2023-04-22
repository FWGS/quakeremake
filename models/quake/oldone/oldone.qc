$modelname "oldone.mdl"
$cd ".\"
$cdtexture "."
$scale 1.0
$origin 0 0 -116 -90

$flags 512

//$cliptotextures

//reference mesh(es)
$bodygroup body
{
studio "reference"
}

$sequence "old" "old" fps 10 loop ACT_IDLE 1
$sequence "shake" "shake" fps 10
$sequence "explode" "explode" fps 10