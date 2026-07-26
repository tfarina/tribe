# Tribe

Tribe is an address book manager for the GNOME and Xfce desktop environments.

## Dependencies

Tribe depends on the following packages:

* [GLib](https://gitlab.gnome.org/GNOME/glib/) >= 2.19.7
* [GTK](https://www.gtk.org) >= 2.16.1

## Build & Install

This project uses [CMake](https://cmake.org/) as its build system.

### 1. Configure the Build

Create a dedicated directory for the build and generate the build files:

```sh
mkdir out && cd out
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**Configuration Options:**

- `-DCMAKE_BUILD_TYPE=Release` — Builds an optimized release version (default). Use `Debug` if you need debug symbols for development or troubleshooting.
- `-DCMAKE_INSTALL_PREFIX=/usr` — Sets the installation prefix. By default, Tribe installs to `/usr/local`. Change this if you want to install to a different location (e.g., `/usr`, `/opt`, or `$HOME/.local`).

**Example: Installing to `/usr` with debug symbols:**

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr ..
```

### 2. Compile the Project

Build the project with:

```sh
make
```

### 3. Install

Install the resulting binaries and other files:

```sh
sudo make install
```

**Note:** The `sudo` command is required if you're installing to system directories like `/usr` or `/usr/local`. If you installed to a user-writable directory (e.g., `$HOME/.local`), you may omit `sudo`.
