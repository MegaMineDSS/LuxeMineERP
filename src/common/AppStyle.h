#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>

class AppStyle {
public:
  static QString getDarkTheme() {
    return R"(
            /* General Application Styling */
            QWidget {
                background-color: #111827; /* Gray 900 */
                color: #F9FAFB;            /* Gray 50 */
                font-family: 'Segoe UI', sans-serif;
                font-size: 11pt;
            }

            /* Main Window & Central Widget Specifics */
            QMainWindow, QWidget#centralwidget {
                background-color: #111827;
            }

            /* Cards / Containers */
            QFrame {
                background-color: #1F2937; /* Gray 800 */
                border: 1px solid #374151; /* Gray 700 */
                border-radius: 8px;
            }

            QGroupBox {
                background-color: #1F2937; /* Gray 800 */
                border: 1px solid #374151; /* Gray 700 */
                border-radius: 8px;
                margin-top: 24px; /* Space for title */
                padding-top: 10px;
            }

            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 5px;
                left: 10px;
                color: #3B82F6; /* Blue 500 */
                font-weight: bold;
            }

            /* Labels */
            QLabel {
                color: #F3F4F6; /* Gray 100 */
            }
            QLabel#headingLabel {
                font-size: 24px;
                font-weight: bold;
                color: #3B82F6; /* Blue 500 */
            }

            /* Line Edits (Inputs) */
            QLineEdit {
                background-color: #374151; /* Gray 700 */
                border: 1px solid #4B5563; /* Gray 600 */
                border-radius: 6px;
                padding: 1px;
                color: #FFFFFF;
                selection-background-color: #3B82F6; /* Blue 500 */
                selection-color: #FFFFFF;
            }
            QLineEdit:focus {
                border: 2px solid #3B82F6; /* Blue 500 border on focus */
                background-color: #1F2937; /* Gray 800 */
            }

            /* Push Buttons - Primary */
            QPushButton {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3B82F6, stop:1 #2563EB); /* Blue 500 to Blue 600 */
                color: #FFFFFF;
                border: none;
                border-radius: 6px;
                padding: 1px 10px;
                font-weight: bold;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #60A5FA, stop:1 #3B82F6); /* Blue 400 to Blue 500 */
            }
            QPushButton:pressed {
                background-color: #1D4ED8; /* Blue 700 */
                padding: 2px 9px; /* Slight pressed effect */
            }
            QPushButton:disabled {
                background-color: #374151; /* Gray 700 */
                color: #9CA3AF; /* Gray 400 */
            }

            /* Secondary / Outline Buttons (Optional) */
            QPushButton[class="secondary"] {
                background-color: transparent;
                border: 1px solid #3B82F6;
                color: #3B82F6;
            }

            /* Checkboxes */
            QCheckBox {
                spacing: 8px;
                color: #E5E7EB; /* Gray 200 */
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
                border: 1px solid #6B7280; /* Gray 500 */
                border-radius: 4px;
                background-color: #374151; /* Gray 700 */
            }
            QCheckBox::indicator:unchecked:hover {
                border-color: #3B82F6;
            }
            QCheckBox::indicator:checked {
                background-color: #3B82F6;
                border-color: #3B82F6;
            }

            /* Status Bar */
            QStatusBar {
                background-color: #1F2937;
                color: #9CA3AF;
                border-top: 1px solid #374151;
            }

            /* Menu Bar */
            QMenuBar {
                background-color: #1F2937;
                color: #E5E7EB;
            }
            QMenuBar::item {
                background-color: transparent;
                padding: 1px 6px;
            }
            QMenuBar::item:selected {
                background-color: #374151;
                color: #3B82F6;
            }

            /* Scrollbars */
            QScrollBar:vertical {
                border: none;
                background: #1F2937;
                width: 10px;
                margin: 0px 0px 0px 0px;
            }
            QScrollBar::handle:vertical {
                background: #4B5563;
                min-height: 20px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical:hover {
                background: #6B7280;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }

            /* Tables */
            QTableWidget {
                background-color: #1F2937; /* Gray 800 */
                gridline-color: #374151;   /* Gray 700 */
                border: 1px solid #374151;
                color: #F3F4F6;
                selection-background-color: #374151; /* Gray 700 selection */
                selection-color: #3B82F6; /* Blue text on selection */
            }
            QHeaderView::section {
                background-color: #111827; /* Gray 900 */
                color: #3B82F6; /* Blue 500 */
                border: none;
                border-bottom: 2px solid #3B82F6;
                padding: 1px;
                font-weight: bold;
            }
        )";
  }
};

#endif // APPSTYLE_H
