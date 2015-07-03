RELEASEDIR=releases/$RELEASE
RELEASESUBDIR=$RELEASEDIR/build-$RELEASE
rm -rf $RELEASEDIR

echo "Releasing $RELEASE"
echo "Exporting global files with ${RELEASE_EXTS}"
mkdir -p $RELEASESUBDIR

eval cp build-$RELEASE/*.{${RELEASE_EXTS}} $RELEASESUBDIR

cp -r build-$RELEASE/ffi $RELEASESUBDIR
cp -r build-$RELEASE/bin $RELEASESUBDIR
cp -r assets $RELEASEDIR
cp -r shaders $RELEASEDIR
cp -r lua $RELEASEDIR

cd $RELEASEDIR
zip -qr ../release-${RELEASE}.zip *


