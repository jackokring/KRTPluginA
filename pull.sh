#!/bin/bash
echo "Pull of active repositories VCV automake"
echo "Use 'pull.sh [dist | arch]' to push dists to drive or make .zip archive"
export TZ="UTC"
datefile=$(date +"%Y-%m-%d-%H-%M")
archive="lin-arm64-2.2.3"

# Published by first
cp ~/bin/pull.sh /mnt/chromeos/GoogleDrive/MyDrive/VCV

# (venv) jackokring@penguin:~/Rack/plugins$ ls */CMakeLists.txt
# Prefabs/CMakeLists.txt  StudioSixPlusOne/CMakeLists.txt

# Reverted to non-building plugins for any not listed to save build time and space

newest() {
	find $1 -type f -printf "%T@ %p\n" | sort -nr | cut -d' ' -f 2- | head -n 1
}

pushd ~/Rack/plugins
for f in SeasideModular SickoCV StochasticTelegraph Stoermelder-P1 StudioSixPlusOne SubmarineFree Thoms VCV-Prototype Venom voxglitch ZZC \
dbRackModules Geodesics HetrickCV ImpromptuModular MindMeldModular ModScript monome nullpath Prefabs CosineKitty-Sapphire \
alefsbits Bogaudio CountModula KRTPluginA Edge
do
	pushd $f
	echo "j $f"
	echo "PLUGIN SLUG: $f\n" >> ~/AUTOBUILD-ERRORS-$datefile.txt
	git pull
	make > /dev/null 2> >(tee -a ~/AUTOBUILD-ERRORS-$datefile.txt >&2)
	if [[ "$1" == "dist" || "$1" == "arch" ]]
	then
		# A make .PHONY (always done) and action not defered by actaul make entry (.PHONY):(real)\n	(null)
		# And unfortunately all: $(TARGET) is done, but has no action and $(TARGET): has a rule sort of, and no variables on the left hand side
		# also resources ... [[]] for shell || form escape to logical form
		if [[ "$(echo plugin-*)" -nt "$(echo dist/*/plugin-*)" || "$(newest res)" -nt "$(echo dist/*/plugin-*)" ]]
		then 
			make dist
			# Option to update newer only?
			cp -u dist/*.vcvplugin /mnt/chromeos/GoogleDrive/MyDrive/VCV
		fi
	fi
	popd
	cp ~/AUTOBUILD-ERRORS-$datefile.txt /mnt/chromeos/GoogleDrive/MyDrive/VCV
done
popd
if [ "$1" == "arch" ]
then
	# Rack build still manual
	pushd /mnt/chromeos/GoogleDrive/MyDrive/VCV
	zip Rack-$archive-$datefile-Archive.zip *.vcvplugin *.vcv *.png RackFree*.zip Rack-SDK*.zip *.md *.txt *.guess *.sh
	popd
fi
echo "end pull"

echo "Pull of active repositories no automake"
pushd ~/Rack/plugins
for f in PS-PurrSoftware
do
	pushd $f
	echo "j $f"
	git pull
	popd
done
popd
echo "end pull"

