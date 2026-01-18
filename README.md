# Quadmastest_Refactoring

# 🐄 Quadmastest - Mastitis Detection Device

> An ESP32-based device to detect mastitis in dairy cattle by measuring milk electrical conductivity. This project helps dairy farmers detect mastitis early.  Mastitis is a common udder infection in cows that reduces milk quality and quantity. 

---

### Our Contributions 🛠️

- **Code Refactoring:** Restructured the codebase into modular components for better maintainability
- **Dashboard Improvements:** Enhanced the web-based visualization interface
  - Added 7-day trend charts with Chart.js
  - Implemented test visualization modal with navigation arrows
  - Added WiFi settings management page
  - Improved file manager with download/delete all features

### How It Works

1. Pour milk from each udder quarter into 4 chambers
2. Device measures **Electrical Conductivity (EC)** of milk
3. Higher EC = possible infection
4. Results shown on display with color coding:
   - 🟢 **Green** = Normal
   - 🟡 **Yellow** = Sub-Clinical (early warning)
   - 🔴 **Red** = Clinical (needs attention)

---
