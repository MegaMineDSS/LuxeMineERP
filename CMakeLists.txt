cmake_minimum_required(VERSION 3.19)

# -------------------------------------------------
# Project
# -------------------------------------------------
project(LuxeMineERP
    VERSION 1.0.0
    LANGUAGES CXX
)

# -------------------------------------------------
# Qt setup (Qt 6 recommended way)
# -------------------------------------------------
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Sql
)

qt_standard_project_setup()

# Tell AUTOUIC where to find .ui files
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

# -------------------------------------------------
# Sources
# -------------------------------------------------
set(SOURCES
    src/main.cpp
    # # ---- Auth ----
    src/auth/LoginWindow.cpp
    src/auth/AuthService.cpp

    # # ---- Dashboards ----
    src/dashboards/AdminWindow.cpp
    src/dashboards/SellerWindow.cpp
    src/dashboards/DesignerWindow.cpp
    src/dashboards/ManufacturerWindow.cpp
    src/dashboards/AccountantWindow.cpp

    # # ---- Database ----
    src/database/DatabaseManager.cpp
    src/database/UserRepository.cpp
    src/database/databaseutils.cpp

    # # ---- Common ----
    src/common/SessionManager.cpp

    # # ---- Admin ----
    src/admin/usercreationwidget.cpp
    src/admin/viewuserswidget.cpp
    src/admin/changepassworddialog.cpp

    # # ---- Seller ----
    src/seller/orderdialog.cpp
)

set(HEADERS
    # ---- Auth ----
    src/auth/LoginWindow.h
    src/auth/AuthService.h

    # # ---- Dashboards ----
    src/dashboards/AdminWindow.h
    src/dashboards/SellerWindow.h
    src/dashboards/DesignerWindow.h
    src/dashboards/ManufacturerWindow.h
    src/dashboards/AccountantWindow.h

    # # ---- Database ----
    src/database/DatabaseManager.h
    src/database/UserRepository.h
    src/database/databaseutils.h

    # # ---- Models ----
    src/models/User.h
    src/models/Order.h
    src/models/UserPaymentView.h
    src/models/OrderData.h
    src/models/OrderStatusData.h

    # # ---- Common ----
    src/common/SessionManager.h
    src/common/Role.h

    # # ---- Admin ----
    src/admin/usercreationwidget.h
    src/admin/viewuserswidget.h
    src/admin/changepassworddialog.h

    # # ---- Seller ----
    src/seller/orderdialog.h
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
)

# -------------------------------------------------
# Executable
# -------------------------------------------------
qt_add_executable(LuxeMineERP
    WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}

    # # ---- not in any use ----
    src/models/imageclicklabel.h src/models/imageclicklabel.cpp
)

# -------------------------------------------------
# Link Qt libraries
# -------------------------------------------------
target_link_libraries(LuxeMineERP
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
)

# -------------------------------------------------
# Include directories
# -------------------------------------------------
target_include_directories(LuxeMineERP PRIVATE
    src
)

# -------------------------------------------------
# Compiler settings (MSVC)
# -------------------------------------------------
if (MSVC)
    target_compile_options(LuxeMineERP PRIVATE /W4)
endif()

# -------------------------------------------------
# Install & Deploy (Qt 6)
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
