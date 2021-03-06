#include <gtest/gtest.h>

#include <android-dl.h>

typedef int (*func_t)();

TEST(AndroidDlTest, OpenNonExistent)
{
    // This shared object does not exist
    ASSERT_EQ(NULL, android_dlopen("nonexistent.so"));
    
    const char *error = android_dl_get_last_error();
    EXPECT_STREQ("android_dlopen: Library nonexistent.so not found", error);
}

TEST(AndroidDlTest, NeedsTest)
{
    const char* library_full_path = "/data/local/tmp/libandroid-dl.so";

    char ** needs = android_dlneeds(library_full_path);
    ASSERT_TRUE(needs != NULL);
    
#if 0
    printf("Library %s\n", library_full_path);
    for (char ** n = needs; *n != NULL; ++n)
        printf("\tNEEDS %s\n", *n);
#endif

    int i = -1;
    EXPECT_STREQ("libc++_shared.so", needs[++i]);
    EXPECT_STREQ("liblog.so", needs[++i]);
    EXPECT_STREQ("libc.so", needs[++i]);
    EXPECT_STREQ("libm.so", needs[++i]);
    EXPECT_STREQ("libstdc++.so", needs[++i]);
    EXPECT_STREQ("libdl.so", needs[++i]);
    EXPECT_TRUE(needs[++i] == NULL);
}

TEST(AndroidDlTest, OpenAbsolutePath)
{
    void * lib = android_dlopen("/system/lib/libxml.so");
    ASSERT_TRUE(lib != NULL);
}

TEST(AndroidDlTest, OpenSystemLib)
{
	// At this point, AndroidDl.initialize was not yet called,
	// so we should be basically emulating dlopen

    void * lib = android_dlopen("libz.so");
    ASSERT_TRUE(lib != NULL);
}

TEST(AndroidDlTest, OpenTest)
{
    void * lib = android_dlopen("libandroid-dl_testlib.so");
    ASSERT_TRUE(lib != NULL);
    
    // This function does not exist in the compilation unit
    EXPECT_EQ(NULL, android_dlsym(lib, "nonexistentfunc"));
    
    // This function is hidden in the compilation unit
    EXPECT_EQ(NULL, android_dlsym(lib, "hiddenfunc"));

    // This function exists
    func_t testfunc = (func_t)android_dlsym(lib, "testfunc");
    ASSERT_TRUE(testfunc != NULL);
    
    // This function returns 42
    EXPECT_TRUE(testfunc() == 42);
    
    // testfunc's address is known
    Dl_info info;
    ASSERT_NE(0, android_dladdr((void*)testfunc, &info));
    EXPECT_STREQ(info.dli_fname, "/data/local/tmp/libandroid-dl_testlib.so");
    EXPECT_STREQ(info.dli_sname, "testfunc");
    EXPECT_EQ(info.dli_saddr, testfunc);

    // testfunc + 1 is still within testfunc
    ASSERT_NE(0, android_dladdr((char*)testfunc + 1, &info));
    EXPECT_STREQ(info.dli_fname, "/data/local/tmp/libandroid-dl_testlib.so");
    EXPECT_STREQ(info.dli_sname, "testfunc");
    EXPECT_EQ(info.dli_saddr, testfunc);
    
    // NOTE: currently android_dlclose does nothing
    EXPECT_EQ(0, android_dlclose(lib));
}
