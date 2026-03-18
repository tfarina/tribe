#include "about_dialog.h"

#include <gtk/gtk.h>

#include "config.h"

void show_about_dialog(GtkWindow *parent) {
  char *license;

  /* List of authors */
  static char const *authors[] = {
    NULL
  };

  /* List of documentation writers */
  static char const *documenters[] = {
    NULL
  };

  static char const copyright[] = "Copyright © 2019";

  static char const *license_part[] = {
      "This is free software; you can redistribute it and/or modify\n"
      "it under the terms of the GNU General Public License as published by\n"
      "the Free Software Foundation; either version 2 of the License, or\n"
      "(at your option) any later version.\n",
      "This is distributed in the hope that it will be useful,\n"
      "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
      "GNU General Public License for more details.\n",
      "You should have received a copy of the GNU General Public License\n"
      "along with this program; if not, see <https://www.gnu.org/licenses/>.\n"
  };

  license = g_strconcat(license_part[0], "\n",
			license_part[1], "\n",
			license_part[2], "\n",
			NULL);

  gtk_show_about_dialog(parent,
                        "program-name",       "Tribe",
			"version",            VERSION,
			"comments",           "A simple and easy to use address book manager",
			"copyright",          copyright,
			"authors",            authors,
			"documenters",        documenters,
			"translator-credits", "",
			"license",            license,
			"website-label",      "Tribe Website",
			"website",            "https://tribe.infinityfreeapp.com",
			NULL);

  g_free(license);
}
