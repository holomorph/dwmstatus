pkgname=dwmstatus-hm-git
pkgver=28.68962ca
pkgrel=1
pkgdesc="A dwm status bar written in C, fork by holomorph"
arch=('i686' 'x86_64')
url="https://github.com/holomorph/dwmstatus"
license=('MIT')
depends=('libmpdclient')
makedepends=('git')
provides=('dwmstatus')
conflicts=('dwmstatus')
source=('git://github.com/holomorph/dwmstatus.git')
md5sums=('SKIP')

pkgver() {
	cd "dwmstatus"
	echo "$(git rev-list --count HEAD).$(git describe --always )"
}

build() {
	make -C "dwmstatus"
}

package() {
	make -C "dwmstatus" PREFIX=/usr DESTDIR="$pkgdir" install
}
