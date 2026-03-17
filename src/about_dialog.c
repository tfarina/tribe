#include "about_dialog.h"

#include <gtk/gtk.h>

#include "config.h"

static char const license_gpl2[] =
    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation; either version 2 of the License, or\n"
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program. If not, see <https://www.gnu.org/licenses/>.";

void show_about_dialog(GtkWindow *parent) {
  /* List of authors */
  static char const *authors[] = {
    NULL
  };

  /* List of documentation writers */
  static char const *documenters[] = {
    NULL
  };

  static char const comments[] = "A simple and easy to use address book manager";
  static char const copyright[] = "Copyright © 2019";

  gtk_show_about_dialog(parent,
                        "program-name",       "Tribe",
			"version",            VERSION,
			"comments",           comments,
			"copyright",          copyright,
			"authors",            authors,
			"documenters",        documenters,
			"translator-credits", "",
			"license",            license_gpl2,
			"website",            PACKAGE_URL,
			NULL);
}
