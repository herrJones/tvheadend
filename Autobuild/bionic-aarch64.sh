source Autobuild/aarch64.sh

if [ -f /etc/nv_tegra_release ]; then
  echo "identified NVidia Jetson"
  AUTOBUILD_CONFIGURE_EXTRA="${AUTOBUILD_CONFIGURE_EXTRA} --enable-jetsonnano --disable-ffmpeg_static"

  echo " ** ${AUTOBUILD_CONFIGURE_EXTRA}"
fi
source Autobuild/bionic.sh

