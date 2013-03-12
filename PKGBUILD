pkgname=dwmstatus-hm-git
pkgver=20130312
pkgrel=1
pkgdesc="A dwm status bar written in C, fork by holomorph"
arch=('i686' 'x86_64')
url="https://github.com/holomorph/dwmstatus"
license=('MIT')
depends=('libmpdclient')
makedepends=('git')
provides=('dwmstatus')
conflicts=('dwmstatus')

_gitroot="https://github.com/holomorph/dwmstatus.git"
_gitname="dwmstatus"

build() {
	cd "$srcdir"
	msg "Connecting to GIT server...."

  if [[ -d $_gitname ]]; then
		cd "$_gitname" && git pull origin
		msg "The local files are updated."
	else
		git clone "$_gitroot" "$_gitname"
	fi

	cd "$srcdir/$_gitname"

	msg "GIT checkout done or server timeout"
	msg "Starting make..."

	make
}

package() {
	cd "$srcdir/$_gitname"
	make PREFIX=/usr DESTDIR="$pkgdir" install
	install -Dm755 dwmvolume "$pkgdir/usr/bin/dwmvolume"
}
