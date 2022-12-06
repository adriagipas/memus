# memus
Els meus emuladors (en realitat simuladors) de maquinetes antigues.

*memus* conté diferents simuladors, cadascun d'ell s'instal·la com un
binari autocontingut. La llista completa de màquines simulades i els
seus corresponents binaris són:

- **Donald Knuth's hypothetical computer MIX**: *memumix*
- **Game Gear**: *memugg*
- **Game Boy Color**: *memugbc*
- **Nintendo Entertainment System**: *memunes*
- **PlayStation**: *memups*

## Compilació i instal·lació

Actualment les instruccions per assumeixen un sistema Linux o
semblant.

Programari requerit:
- git
- [meson](https://mesonbuild.com/)

Biblioteques de programari requerides:
- [GLib 2.0](https://gitlab.gnome.org/GNOME/glib/)
- [SDL 2.0](https://github.com/libsdl-org/SDL)
- [SDL_image 2.0](https://github.com/libsdl-org/SDL_image)
- [dbus 1.14](https://gitlab.freedesktop.org/dbus/dbus)

El primer pas és descarregar el codi font (incloent tots els submòduls):
```
git clone --recurse-submodules https://github.com/adriagipas/memus.git
cd memus
```

Després, dins de la carpeta del programari, emprem *meson* per
compilar i instal·lar els simuladors:
```
meson setup --buildtype release --prefix $(INSTALLATION_PREFIX) build
cd build
meson compile
meson install
```
