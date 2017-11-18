#ifndef __CHECK_CFG_H__
#define __CHECK_CFG_H__

#define TEST_EMPTYSTR(setter)	do {					\
	const char	*errstr;					\
									\
	errstr = NULL;							\
	ck_assert_int_eq(setter(NULL, NULL), -1); 			\
	ck_assert_int_eq(setter(NULL, &errstr), -1); 			\
	ck_assert_str_eq(errstr, "empty");				\
									\
	errstr = NULL;							\
	ck_assert_int_eq(setter("", &errstr), -1);			\
	ck_assert_str_eq(errstr, "empty");				\
} while (0)

#define TEST_EMPTYSTR_T(type, objget, list, setter)	do {		\
	type		obj3 = objget(list, #setter);			\
	const char	*errstr;					\
									\
	errstr = NULL;							\
	ck_assert_int_eq(setter(obj3, list, NULL, NULL), -1);		\
	ck_assert_int_eq(setter(obj3, list, NULL, &errstr), -1);	\
	ck_assert_str_eq(errstr, "empty");				\
									\
	errstr = NULL;							\
	ck_assert_int_eq(setter(obj3, list, "", &errstr), -1);		\
	ck_assert_str_eq(errstr, "empty");				\
} while (0)

#define TEST_XSTRDUP(setter, getter)	do {				\
	TEST_EMPTYSTR(setter);						\
									\
	ck_assert_int_eq(setter("check_cfg", NULL), 0); 		\
	ck_assert_str_eq(getter(), "check_cfg");			\
} while (0)

#define TEST_XSTRDUP_T(type, objget, list, setter, getter) do {		\
	type		obj2 = objget(list, #setter);			\
									\
	TEST_EMPTYSTR_T(type, objget, list, setter);			\
									\
	ck_assert_int_eq(setter(obj2, list, "check_cfg", NULL), 0);	\
	ck_assert_str_eq(getter(obj2), "check_cfg");			\
} while (0)

#define TEST_STRLCPY(setter, getter, length)	do {			\
	char		 buf[length + 1];				\
	const char	*errstr2;					\
									\
	TEST_XSTRDUP(setter, getter);					\
									\
	errstr2 = NULL; 						\
	memset(buf, 'A', sizeof(buf) - 1);				\
	buf[sizeof(buf) - 1] = '\0';					\
	ck_assert_int_eq(setter(buf, NULL), -1);			\
	ck_assert_int_eq(setter(buf, &errstr2), -1);			\
	ck_assert_ptr_ne(errstr2, NULL);				\
} while (0)

#define TEST_STRLCPY_T(type, objget, list, setter, getter, length) do { \
	type		 obj = objget(list, #setter);			\
	char		 buf[length + 1];				\
	const char	*errstr2;					\
									\
	TEST_XSTRDUP_T(type, objget, list, setter, getter);		\
									\
	errstr2 = NULL; 						\
	memset(buf, 'A', sizeof(buf) - 1);				\
	buf[sizeof(buf) - 1] = '\0';					\
	ck_assert_int_eq(setter(obj, list, buf, NULL), -1);		\
	ck_assert_int_eq(setter(obj, list, buf, &errstr2), -1); 	\
	ck_assert_ptr_ne(errstr2, NULL);				\
} while (0)

#define TEST_BOOLEAN(setter, getter)	do {				\
	const char	*errstr2;					\
									\
	TEST_EMPTYSTR(setter);						\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter("BOGUS", NULL), -1);			\
	ck_assert_int_eq(setter("BOGUS", &errstr2), -1);		\
	ck_assert_str_eq(errstr2, "invalid");				\
									\
	ck_assert_int_eq(setter("tRuE", NULL), 0);			\
	ck_assert_int_eq(getter(), 1);					\
	ck_assert_int_eq(setter("YeS", NULL), 0);			\
	ck_assert_int_eq(getter(), 1);					\
	ck_assert_int_eq(setter("1", NULL), 0);				\
	ck_assert_int_eq(getter(), 1);					\
									\
	ck_assert_int_eq(setter("FaLsE", NULL), 0);			\
	ck_assert_int_eq(getter(), 0);					\
	ck_assert_int_eq(setter("nO", NULL), 0);			\
	ck_assert_int_eq(getter(), 0);					\
	ck_assert_int_eq(setter("0", NULL), 0);				\
	ck_assert_int_eq(getter(), 0);					\
} while (0)

#define TEST_BOOLEAN_T(type, objget, list, setter, getter) do { 	\
	type		 obj = objget(list, #setter);			\
	const char	*errstr2;					\
									\
	TEST_EMPTYSTR_T(type, objget, list, setter);			\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter(obj, list, "BOGUS", NULL), -1); 	\
	ck_assert_int_eq(setter(obj, list, "BOGUS", &errstr2), -1);	\
	ck_assert_str_eq(errstr2, "invalid");				\
									\
	ck_assert_int_eq(setter(obj, list, "tRuE", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 1);				\
	ck_assert_int_eq(setter(obj, list, "YeS", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 1);				\
	ck_assert_int_eq(setter(obj, list, "1", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 1);				\
									\
	ck_assert_int_eq(setter(obj, list, "FaLsE", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 0);				\
	ck_assert_int_eq(setter(obj, list, "nO", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 0);				\
	ck_assert_int_eq(setter(obj, list, "0", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 0);				\
} while (0)

#define TEST_UINTNUM(setter, getter)	do {				\
	const char	*errstr2;					\
									\
	TEST_EMPTYSTR(setter);						\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter("-1", NULL), -1);			\
	ck_assert_int_eq(setter("-1", &errstr2), -1);			\
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter("4294967296", &errstr2), -1);		\
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	ck_assert_int_eq(setter("20", NULL), 0);			\
	ck_assert_uint_eq(getter(), 20);				\
} while (0)

#define TEST_UINTNUM_T(type, objget, list, setter, getter)	do {	\
	type		 obj = objget(list, #setter);			\
	const char	*errstr2;					\
									\
	TEST_EMPTYSTR_T(type, objget, list, setter);			\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter(obj, list, "-1", NULL), -1);		\
	ck_assert_int_eq(setter(obj, list, "-1", &errstr2), -1);	\
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter(obj, list, "4294967296", &errstr2), -1); \
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	ck_assert_int_eq(setter(obj, list, "20", NULL), 0);		\
	ck_assert_uint_eq(getter(obj), 20);				\
} while (0)

#define TEST_INTNUM(setter, getter)	do {				\
	const char	*errstr2;					\
									\
	TEST_EMPTYSTR(setter);						\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter("-2147483649", NULL), -1);		\
	ck_assert_int_eq(setter("-2147483649", &errstr2), -1);		\
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter("2147483648", &errstr2), -1);		\
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	ck_assert_int_eq(setter("20", NULL), 0);			\
	ck_assert_int_eq(getter(), 20);					\
	ck_assert_int_eq(setter("-20", NULL), 0);			\
	ck_assert_int_eq(getter(), -20);				\
} while (0)

#define TEST_INTNUM_T(type, objget, list, setter, getter)	do {	\
	type		 obj = objget(list, #setter);			\
	const char	*errstr2;					\
									\
	TEST_EMPTYSTR(type, objget, list, setter, getter);		\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter(obj, list, "-2147483649", NULL), -1);	\
	ck_assert_int_eq(setter(obj, list, "-2147483649", &errstr2), -1); \
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	errstr2 = NULL; 						\
	ck_assert_int_eq(setter(obj, list, "2147483648", &errstr2), -1); \
	ck_assert_ptr_ne(errstr2, NULL);				\
									\
	ck_assert_int_eq(setter(obj, list, "20", NULL), 0);		\
	ck_assert_int_eq(getter(obj), 20);				\
	ck_assert_int_eq(setter(obj, list, "-20", NULL), 0);		\
	ck_assert_int_eq(getter(obj), -20);				\
} while (0)

#endif /* __CHECK_CFG_H__ */
