cmake_minimum_required(VERSION 3.19)

# -------------------------------------------------
# Project
# -------------------------------------------------
project(LuxeMineERP
    VERSION 1.0.0
    LANGUAGES CXX
)

# -------------------------------------------------
# Qt setup
# -------------------------------------------------
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Sql
    GuiPrivate
    CorePrivate
)

qt_standard_project_setup()

# Tell AUTOUIC where to find .ui files
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

# -------------------------------------------------
# External: QXlsx
# -------------------------------------------------
file(GLOB QXLSX_SOURCES
    external/QXlsx/source/*.cpp
)

file(GLOB QXLSX_HEADERS
    external/QXlsx/include/*.h
)

# -------------------------------------------------
# Project sources
# -------------------------------------------------
set(SOURCES
    src/main.cpp

    src/auth/LoginWindow.cpp
    src/auth/AuthService.cpp

    src/dashboards/AdminWindow.cpp
    src/dashboards/SellerWindow.cpp
    src/dashboards/DesignerWindow.cpp
    src/dashboards/ManufacturerWindow.cpp
    src/dashboards/AccountantWindow.cpp

    src/database/DatabaseManager.cpp
    src/database/UserRepository.cpp
    src/database/databaseutils.cpp

    src/models/imageclicklabel.cpp

    src/common/SessionManager.cpp
    src/common/rolewindowfactory.cpp
    src/common/switchroledialog.cpp
    src/common/goldweightcalculator.cpp

    src/admin/usercreationwidget.cpp
    src/admin/viewuserswidget.cpp
    src/admin/changepassworddialog.cpp

    src/seller/orderwidget.cpp
    src/seller/orderlistwidget.cpp

    src/accountant/castingwidget.cpp
    src/accountant/castinglistwidget.cpp

    src/dashboards/addcatalog.cpp
    src/dashboards/jewelrymenu.cpp
    src/dashboards/modifycatalog.cpp

    src/manufacturer/jobsheetwidget.cpp
    src/manufacturer/managegolddialog.cpp
    src/manufacturer/diamonissueretbrodialog.cpp
    src/manufacturer/jobslistwidget.cpp
)

set(HEADERS
    src/auth/LoginWindow.h
    src/auth/AuthService.h

    src/dashboards/AdminWindow.h
    src/dashboards/SellerWindow.h
    src/dashboards/DesignerWindow.h
    src/dashboards/ManufacturerWindow.h
    src/dashboards/AccountantWindow.h

    src/database/DatabaseManager.h
    src/database/UserRepository.h
    src/database/databaseutils.h

    src/models/User.h
    src/models/Order.h
    src/models/UserPaymentView.h
    src/models/OrderData.h
    src/models/OrderStatusData.h
    src/models/imageclicklabel.h
    src/models/CastingData.h
    src/models/CastingListRow.h
    src/models/JobSheetData.h

    src/common/SessionManager.h
    src/common/Role.h
    src/common/rolewindowfactory.h
    src/common/switchroledialog.h
    src/common/goldweightcalculator.h

    src/admin/usercreationwidget.h
    src/admin/viewuserswidget.h
    src/admin/changepassworddialog.h

    src/seller/orderwidget.h
    src/seller/orderlistwidget.h

    src/accountant/castingwidget.h
    src/accountant/castinglistwidget.h

    src/dashboards/addcatalog.h
    src/dashboards/jewelrymenu.h
    src/dashboards/modifycatalog.h

    src/manufacturer/jobsheetwidget.h
    src/manufacturer/managegolddialog.h
    src/manufacturer/diamonissueretbrodialog.h
    src/manufacturer/jobslistwidget.h
)

set(UI_FILES
    ui/login.ui
    ui/admin.ui
    ui/seller.ui
    ui/designer.ui
    ui/manufacturer.ui
    ui/accountant.ui
    ui/usercreation.ui
    ui/viewusers.ui
    ui/changepassword.ui
    ui/order.ui
    ui/orderlist.ui
    ui/casting.ui
    ui/castinglist.ui
    ui/jobsheet.ui
    ui/managegold.ui
    ui/diamonissueretbro.ui
    ui/jobslist.ui
    ui/addcatalog.ui
    ui/modifycatalog.ui
)

# -------------------------------------------------
# Executable
# -------------------------------------------------
qt_add_executable(LuxeMineERP
    WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# -------------------------------------------------
# Attach QXlsx to target
# -------------------------------------------------
target_sources(LuxeMineERP PRIVATE
    ${QXLSX_SOURCES}
    ${QXLSX_HEADERS}
)

# -------------------------------------------------
# Include directories
# -------------------------------------------------
target_include_directories(LuxeMineERP PRIVATE
    src
    src/models
    src/common
    src/database
    external/QXlsx/include
)

qt_add_resources(APP_RESOURCES
    resources.qrc
)

target_sources(LuxeMineERP PRIVATE ${APP_RESOURCES})

# -------------------------------------------------
# Link libraries
# -------------------------------------------------
target_link_libraries(LuxeMineERP
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::CorePrivate
)

# -------------------------------------------------
# Compiler settings
# -------------------------------------------------
if (MSVC)
    target_compile_options(LuxeMineERP PRIVATE /W4)
endif()

# -------------------------------------------------
# Install & Deploy
# -------------------------------------------------
include(GNUInstallDirs)

install(TARGETS LuxeMineERP
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET LuxeMineERP
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
