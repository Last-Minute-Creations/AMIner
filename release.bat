@echo off
mkdir game
copy aminer game
copy aminer.info game
xcopy data game\data /e /i /h
mkdir game\s
echo aminer > game\s\startup-sequence
exe2adf aminer -l "AMIner" -a "aminer.adf" -0 -d game
rmdir game /s /q