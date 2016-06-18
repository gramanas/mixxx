#ifndef LIBRARYSIDEBAREXPANDEDMANAGER_H
#define LIBRARYSIDEBAREXPANDEDMANAGER_H
#include "library/librarypanemanager.h"

class LibrarySidebarExpandedManager : public LibraryPaneManager
{
  public:
    LibrarySidebarExpandedManager(QObject* parent = nullptr);

    void bindPaneWidget(WBaseLibrary* libraryWidget,
                        KeyboardEventFilter* pKeyboard) override;
};

#endif // LIBRARYSIDEBAREXPANDEDMANAGER_H
