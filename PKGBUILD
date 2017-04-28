# Maintainer: Jesse McClure AKA "Trilby" <jesse [dot] mcclure [at] umassmed [dot] edu>
_gitname="goldy-gopher"
pkgname="${_gitname}-git"
pkgver=0
pkgrel=1
pkgdesc="A simple gopher client and server"
url=""
arch=('x86_64' 'i686')
license=('GPL3')
makedepends=('git')
source=()
sha256sums=('SKIP')

pkgver() {
	cd "${_gitname}";
	echo "4.$(git rev-list --count HEAD).$(git describe --always )"
}

build() {
	cd "${_gitname}"
	make PREFIX=/usr
}

package() {
	cd "${_gitname}"
	make PREFIX=/usr DESTDIR="${pkgdir}" install
}
