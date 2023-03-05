#!/bin/bash
export TZ="UTC"
# add commit push
gacp () {
	date=$(date +"%A %Y-%m-%d %H:%M:%S")
  message="${1:-$date [dev-bot]}"
  git add . ; git commit -m "$message" ; git push
}
echo "Push of active repositories"
pushd ~
for f in KRTPluginADemo Rack/plugins/KRTPluginA Rack/plugins/KRTPluginJ
do
	pushd $f
	gacp $1
	popd
done
popd
echo "end push"

