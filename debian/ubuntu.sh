CHANGELOG_BAK=/tmp/miam-player_changelog
mkchangelog() {
  cat >debian/changelog<<EOF
miam-player ($1) $2; urgency=low

  * Upstream update

 -- Matthieu Bachelier (MBach) <bachelier.matthieu@gmail.com>  $(LANG=C date -R)

`cat $CHANGELOG_BAK`
EOF
}

DISTRIBUTIONS=(wily)
DATE=`date -d @$(git log -n1 --format="%at") +%Y%m%d-%H%M%S`
for D in ${DISTRIBUTIONS[@]}; do
  git checkout -- debian/changelog
  cp -avf debian/changelog $CHANGELOG_BAK
  VER=0.8.0~`git log -1 --pretty=format:"git${DATE}.%h~${D}" 2> /dev/null`
  mkchangelog $VER $D
  debuild -S -sa -k7CA4455D
  dput -f ppa:bachelier-matthieu/ppa ../miam-player_${VER}_source.changes
  cp -avf $CHANGELOG_BAK debian/changelog
done

