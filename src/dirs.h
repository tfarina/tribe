#ifndef DIRS_H_
#define DIRS_H_

int dirs_init(void);
void dirs_shutdown(void);

char const *dirs_get_user_data_dir(void);

#endif  /* DIRS_H_ */
