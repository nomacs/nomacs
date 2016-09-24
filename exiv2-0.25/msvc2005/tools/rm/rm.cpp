/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* Written in 2011, 2012 by John Ford <jhford@mozilla.com>
 *
 * This program is a replacement for the Posix 'rm' utility implemented as
 * a native Windows win32 application.  Build using accompanying Makefile
 *    make
 * or by running
 *    cl rm.cpp
 */
#include <windows.h>
#include <Strsafe.h>
#include <string.h>
#include <stdio.h>



/* TODO:
 *   -should the wow64fsredirection stuff be applicable to the whole app
 *    or only per empty_directory invocation?
 *   -support simple unix-style paths (i.e. map /c/dir1/file1 to c:\\dir1\\file1)
 *   -return non-zero if no files are deleted and -f isn't specified
 *   -multi-thread deletions
 */

/* This function takes an errNum, filename of the file being operated on and
 * a stdio file handle to the file where output should be printed
 */
void print_error(DWORD errNum, wchar_t* filename, FILE* fhandle){
    wchar_t* msg;
    FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errNum,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR) &msg,
            0, NULL);
    fwprintf(fhandle, L"\"%ws\" - %ws", filename, msg);
}

/* Remove an empty directory.  This will fail if there are still files or
 * other directories in the directory specified by name
 */
BOOL del_directory(wchar_t* name, BOOL force, BOOL verbose, BOOL quiet){
    BOOL rv = TRUE;
    if (verbose) {
        fwprintf(stdout, L"deleting directory \"%ws\"\n", name);
    }
    BOOL delStatus = RemoveDirectoryW(name);
    if (!delStatus) {
        rv = FALSE;
        if (!quiet) {
            print_error(GetLastError(), name, stderr);
        }
    }
    if (verbose) {
        fwprintf(stdout, L"deleted directory \"%ws\"\n", name);
    }
    return rv;
}

/* Remove a file.  If force is true, read only and system file system
 * attributes are cleared before deleting the file
 */
BOOL del_file(wchar_t* name, BOOL force, BOOL verbose, BOOL quiet){
    BOOL rv = TRUE;
    if (force) {
        DWORD fileAttr = GetFileAttributesW(name);
        if (fileAttr == INVALID_FILE_ATTRIBUTES) {
            if (!quiet) {
                fwprintf(stderr, L"invalid file attributes for \"%ws\"\n", name);
            }
            // Hmm, should I still try to delete the file?
            return FALSE;
        }
        if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
            if (!quiet) {
                fwprintf(stderr, L"%ws is a directory, not a file\n", name);
                rv = FALSE;
            }
        }
        // Should really only have one SetFileAttributes
        if (fileAttr & FILE_ATTRIBUTE_SYSTEM ||
            fileAttr & FILE_ATTRIBUTE_READONLY) {
            DWORD toSet = FILE_ATTRIBUTE_NORMAL;
            if (verbose) {
                wprintf(L"changing \"%ws\" file attributes to be removable\n", name);
            }
            DWORD setAttrStatus = SetFileAttributesW(name, toSet);
            if (!setAttrStatus){
                rv = FALSE;
                if (!quiet) {
                    print_error(setAttrStatus, name, stderr);
                }
            }
        }
    }
    if (verbose) {
        fwprintf(stdout, L"deleting \"%ws\"\n", name);
    }
    BOOL delStatus = DeleteFileW(name);
    if (!delStatus) {
        rv = FALSE;
        if (!quiet)
            print_error(GetLastError(), name, stderr);
    } else if (verbose) {
        fwprintf(stdout, L"deleted \"%ws\"\n", name);
    }
    return rv;
}

/* This function will recursively remove all files in a directory
 * then the directory itself.
 */
BOOL empty_directory(wchar_t* name, BOOL force, BOOL verbose, BOOL quiet){
    BOOL rv = TRUE;
    DWORD ffStatus;
    WIN32_FIND_DATAW findFileData;
    // TODO: Don't waste so much memory!
    wchar_t dir[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
	// Used while disabling Wow64 FS Redirection
	//Unused for now PVOID* wow64value = NULL;

    /* without a trailing \*, the listing for "c:\windows" would show info
     * for "c:\windows", not files *inside* of "c:\windows"
     */
    StringCchCopyW(dir, MAX_PATH, name); // TODO: Check return
    StringCchCatW(dir, MAX_PATH, L"\\*");

    /* We don't know what's going on, but Wow64 redirection
     * is not working quite right.  Since nothing we have should
     * be in a location that needs Wow64, we should be fine to
     * ignore it
     */
    //Wow64DisableWow64FsRedirection(wow64value);

    hFind = FindFirstFileW(dir, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        rv = FALSE;
        if (!quiet) {
            print_error(GetLastError(), name, stderr);
        }
        return rv;
    }

    do {
        wchar_t fullName[MAX_PATH];
        StringCchCopyW(fullName, MAX_PATH, name);
        StringCchCatW(fullName, MAX_PATH, L"\\");
        StringCchCatW(fullName, MAX_PATH, findFileData.cFileName);
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (wcscmp(L".", findFileData.cFileName) != 0 && wcscmp(L"..", findFileData.cFileName) != 0){
                if (!empty_directory(fullName, force, verbose, quiet)){
                    rv = FALSE;
                }
            }
        } else {
            if (!del_file(fullName, force, verbose, quiet)) {
                rv = FALSE;
            }
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);

    /* if (!Wow64RevertWow64FsRedirection(wow64value)) {
     *    if (!quiet) {
     *        fwprintf(stderr, L"Error restoring Wow64 FS Redirection\n");
     *    }
     *    return FALSE;
     * }
     */

    ffStatus = GetLastError();
    if (ffStatus != ERROR_NO_MORE_FILES) {
        print_error(ffStatus, findFileData.cFileName, stderr);
        rv = FALSE;
    }

    FindClose(hFind);

    del_directory(name, force, verbose, quiet);

    return rv;

}

/* This function is used to delete a file or directory specified by the
 * 'name' variable.  The type of 'name' is figured out.  If the recurse
 * option is TRUE, directories will be recursively emptied then deleted.
 * If force is TRUE, file attributes will be changed to allow the program
 * to delete the file.  The verbose option will cause non-fatal error messages
 * to print to stderr.  The quiet option will suppress all but fatal
 * error messages
 */
BOOL del(wchar_t* name, BOOL recurse, BOOL force, BOOL verbose, BOOL quiet) {
    BOOL rv = TRUE;
    DWORD fileAttr = GetFileAttributesW(name);
    if (fileAttr == INVALID_FILE_ATTRIBUTES){
        rv = FALSE;
        if (!quiet) {
            fwprintf(stderr, L"Invalid file attributes for \"%ws\"\n", name);
        }
    } else if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
        if (recurse){
            if (!empty_directory(name, force, verbose, quiet)){
                rv = FALSE;
            }
        } else {
            if (!del_directory(name, force, verbose, quiet)){
                rv = FALSE;
            }
        }
    } else {
        if (!del_file(name, force, verbose, quiet)){
            rv = FALSE;
        }
    }
    return rv;
}

/* This struct is used by the command line parser */
struct node{
    node *next;
    wchar_t* data;
};

int wmain(int argc, wchar_t** argv)
{
    int exitCode = 0;
    int i, j;
    BOOL verbose = FALSE, force = FALSE, quiet = FALSE, recurse = FALSE;
    BOOL onlyFiles = FALSE;
    struct node *previous = NULL;
    struct node *start = NULL;
    for (i = 1 ; i < argc ; i++) {
        if (wcscmp(argv[i], L"--") == 0) {
            /* Once we've seen '--' as an arg in the argv,
             * we want to interpret everything after that point
             * as a file
             */
            onlyFiles = TRUE;
        } else if (!onlyFiles && argv[i][0] == L'-') {
            /* Before the -- appears (if ever), we assume that all
             * args starting with - are options.  If I wanted to do
             * full words, I would have a check for the second char
             * being another - in a case and use that case and wsccmp
             * to set the options.
             */
            for (j = 1 ; j < wcslen(argv[i]) ; j++) {
                switch(argv[i][j]){
                    case L'v':
                        verbose = TRUE;
                        break;
                    case L'q':
                        quiet = TRUE;
                        break;
                    case L'r':
                        recurse = TRUE;
                        break;
                    case L'f':
                        force = TRUE;
                        break;
                    default:
                        fwprintf(stderr, L"The option -%wc is not valid\n", argv[i][j]);
                        exitCode = 1;
                 }
            }
        } else {
            /* If there are no more options, or we are forcing the rest of the
             * args to be files, we add them to the linked list.  This list stores
             * args in reverse order to what is on the command line.
             */
            struct node *nextNode = (struct node *) malloc(sizeof(struct node));
            nextNode->data = argv[i];
            nextNode->next = previous;
            previous = nextNode;
            start = nextNode;
        }
    }
    if (verbose && quiet) {
        fwprintf(stderr, L"The -q (quiet) and -v (verbose) options are incompatible\n");
        exitCode = 1;
    }
    /* If everything is good, its time to start deleting the files.
     * We do this by traversing the linked list, deleting the current
     * node then deleting the current node before moving to the next
     */
    if (!exitCode) {
        struct node* current = start;
        while (current != NULL){
            BOOL result = del(current->data, recurse, force, verbose, quiet);
            if (!result) {
                exitCode = 1;
            }
            struct node* cleanup = current;
            current = current->next;
            free(cleanup);
        }
    }

    return exitCode;
}
