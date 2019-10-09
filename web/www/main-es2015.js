(window["webpackJsonp"] = window["webpackJsonp"] || []).push([["main"],{

/***/ "./$$_lazy_route_resource lazy recursive":
/*!******************************************************!*\
  !*** ./$$_lazy_route_resource lazy namespace object ***!
  \******************************************************/
/*! no static exports found */
/***/ (function(module, exports) {

function webpackEmptyAsyncContext(req) {
	// Here Promise.resolve().then() is used instead of new Promise() to prevent
	// uncaught exception popping up in devtools
	return Promise.resolve().then(function() {
		var e = new Error("Cannot find module '" + req + "'");
		e.code = 'MODULE_NOT_FOUND';
		throw e;
	});
}
webpackEmptyAsyncContext.keys = function() { return []; };
webpackEmptyAsyncContext.resolve = webpackEmptyAsyncContext;
module.exports = webpackEmptyAsyncContext;
webpackEmptyAsyncContext.id = "./$$_lazy_route_resource lazy recursive";

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/app.component.html":
/*!**************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/app.component.html ***!
  \**************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-drawer-container class=\"root-container mat-background\" [hasBackdrop]=\"true\">\r\n  <mat-drawer #toolBar [mode]=\"side\" (click)=\"toolBar.toggle()\">\r\n    <mat-toolbar class=\"main-toolbar mat-primary\">\r\n      <button mat-icon-button matTooltip=\"Hide main menu\" [matTooltipShowDelay]=\"500\">\r\n        <mat-icon>menu</mat-icon>\r\n      </button>\r\n      <span>{{title}}</span>\r\n    </mat-toolbar>\r\n    <mat-nav-list>\r\n      <a mat-list-item [routerLink]=\"'/'\"> Dashboard </a>\r\n      <a mat-list-item [routerLink]=\"'/Preferences'\" routerLinkActive=\"active\"> Preferences </a>\r\n      <a mat-list-item [routerLink]=\"'/Roles'\" routerLinkActive=\"active\"> Roles </a>\r\n      <a mat-list-item [routerLink]=\"'/Users'\" routerLinkActive=\"active\"> Users </a>\r\n      <a mat-list-item [routerLink]=\"'/Interfaces'\" routerLinkActive=\"active\"> Interfaces </a>\r\n      <a mat-list-item [routerLink]=\"'/TimerPlans'\" routerLinkActive=\"active\"> Timer Plans </a>\r\n      <a mat-list-item [routerLink]=\"'/Units'\" routerLinkActive=\"active\"> Units </a>\r\n      <a mat-list-item [routerLink]=\"'/Scenes'\" routerLinkActive=\"active\"> Scenes </a>\r\n      <a mat-list-item [routerLink]=\"'/StandardScripts'\" routerLinkActive=\"active\"> Standard Scripts </a>\r\n    </mat-nav-list>\r\n  </mat-drawer>\r\n  <mat-drawer-content>\r\n    <mat-toolbar class=\"main-toolbar mat-primary\">\r\n      <button mat-icon-button (click)=\"toolBar.toggle()\" matTooltip=\"Show main menu\" [matTooltipShowDelay]=\"500\">\r\n        <mat-icon>menu</mat-icon>\r\n      </button>\r\n      <span>Today</span>\r\n      <button mat-icon-button [matMenuTriggerFor]=\"userMenu\" matTooltip=\"Show user menu\" [matTooltipShowDelay]=\"500\">\r\n        <mat-icon>person</mat-icon>\r\n      </button>\r\n    </mat-toolbar>\r\n    <mat-menu #userMenu=\"matMenu\">\r\n      <button mat-menu-item>Profile</button>\r\n      <button mat-menu-item>Theme</button>\r\n      <button mat-menu-item>Logoff</button>\r\n    </mat-menu>    <router-outlet></router-outlet>\r\n  </mat-drawer-content>\r\n</mat-drawer-container>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/element-list/element-list.component.html":
/*!**********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/element-list/element-list.component.html ***!
  \**********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>element-list works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/element/element.component.html":
/*!************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/element/element.component.html ***!
  \************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>element works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/layout-list/layout-list.component.html":
/*!********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/layout-list/layout-list.component.html ***!
  \********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-tab-group>\r\n  <mat-tab label=\"Cards\">\r\n    <mat-card class=\"dmz-top-level-card\">\r\n      <mat-card-header>\r\n        <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n        <mat-card-title>Cards are the best</mat-card-title>\r\n        <mat-card-subtitle>Really, they are!</mat-card-subtitle>\r\n      </mat-card-header>\r\n      <img mat-card-image src=\"https://source.unsplash.com/random/800x600\">\r\n      <mat-card-content>\r\n        <p>Something pretty...</p>\r\n      </mat-card-content>\r\n    </mat-card>\r\n  </mat-tab>\r\n  <mat-tab label=\"Controls\">\r\n    <mat-form-field>\r\n      <mat-label>Sidenav mode</mat-label>\r\n      <mat-select #mode>\r\n        <mat-option value=\"side\">Side</mat-option>\r\n        <mat-option value=\"over\">Over</mat-option>\r\n        <mat-option value=\"push\">Push</mat-option>\r\n      </mat-select>\r\n    </mat-form-field>\r\n    <mat-form-field>\r\n      <mat-label>Has backdrop</mat-label>\r\n      <mat-select #hasBackdrop>\r\n        <mat-option>Unset</mat-option>\r\n        <mat-option [value]=\"true\">True</mat-option>\r\n        <mat-option [value]=\"false\">False</mat-option>\r\n      </mat-select>\r\n    </mat-form-field>\r\n    <button mat-raised-button (click)=\"toolBar.toggle()\">Toggle drawer</button>\r\n    <mat-list _nghost-wkf-c17=\"\" class=\"ng-star-inserted\">\r\n      <h2 _ngcontent-wkf-c17=\"\">Unordered List</h2>\r\n      <ul _ngcontent-wkf-c17=\"\" class=\"items\">\r\n        <li _ngcontent-wkf-c17=\"\" class=\"ng-star-inserted\"><a _ngcontent-wkf-c17=\"\" ng-reflect-router-link=\"/Roles,1\" href=\"/Roles/1\"><span _ngcontent-wkf-c17=\"\" class=\"badge\">1</span>Blah Data 1 </a></li>\r\n        <li _ngcontent-wkf-c17=\"\" class=\"ng-star-inserted\"><a _ngcontent-wkf-c17=\"\" ng-reflect-router-link=\"/Roles,2\" href=\"/Roles/2\"><span _ngcontent-wkf-c17=\"\" class=\"badge\">2</span>Blah Data 2 </a></li>\r\n        <li _ngcontent-wkf-c17=\"\" class=\"ng-star-inserted\"><a _ngcontent-wkf-c17=\"\" ng-reflect-router-link=\"/Roles,3\" href=\"/Roles/3\"><span _ngcontent-wkf-c17=\"\" class=\"badge\">3</span>Blah Data 3 </a></li>\r\n      </ul>\r\n    </mat-list>\r\n  </mat-tab>\r\n  <mat-tab label=\"SVG Samples\">\r\n    <mat-card>\r\n      <mat-icon>person</mat-icon>\r\n      <mat-icon>people</mat-icon>\r\n      <mat-icon>person_outline</mat-icon>\r\n      <mat-icon>add_location</mat-icon>\r\n      <mat-icon>my_location</mat-icon>\r\n      <mat-icon>alarm</mat-icon>\r\n      <mat-icon>alarm_add</mat-icon>\r\n      <mat-icon>alarm_off</mat-icon>\r\n      <mat-icon>alarm_on</mat-icon>\r\n      <mat-icon>backup</mat-icon>\r\n      <mat-icon>build</mat-icon>\r\n      <mat-icon>calendar_today</mat-icon>\r\n      <mat-icon>check_circle</mat-icon>\r\n      <mat-icon>date_range</mat-icon>\r\n      <mat-icon>dashboard</mat-icon>\r\n      <mat-icon>delete</mat-icon>\r\n      <mat-icon>delete_forever</mat-icon>\r\n      <mat-icon>done</mat-icon>\r\n      <mat-icon>drag_indicator</mat-icon>\r\n      <mat-icon>event</mat-icon>\r\n      <mat-icon>exit_to_app</mat-icon>\r\n      <mat-icon>extension</mat-icon>\r\n      <mat-icon>get_app</mat-icon>\r\n      <mat-icon>help</mat-icon>\r\n      <mat-icon>history</mat-icon>\r\n      <mat-icon>home</mat-icon>\r\n      <mat-icon>horizontal_split</mat-icon>\r\n      <mat-icon>input</mat-icon>\r\n      <mat-icon>list</mat-icon>\r\n      <mat-icon>restore</mat-icon>\r\n      <mat-icon>schedule</mat-icon>\r\n      <mat-icon>tab</mat-icon>\r\n      <mat-icon>tab_unselected</mat-icon>\r\n      <mat-icon>timeline</mat-icon>\r\n      <mat-icon>today</mat-icon>\r\n      <mat-icon>track_changes</mat-icon>\r\n      <mat-icon>update</mat-icon>\r\n      <mat-icon>zoom_in</mat-icon>\r\n      <mat-icon>zoom_out</mat-icon>\r\n    </mat-card><br />\r\n    <mat-card>\r\n      <mat-icon>add_to_queue</mat-icon>\r\n      <mat-icon>airplay</mat-icon>\r\n      <mat-icon>cast</mat-icon>\r\n      <mat-icon>fast_forward</mat-icon>\r\n      <mat-icon>fast_rewind</mat-icon>\r\n      <mat-icon>forward_10</mat-icon>\r\n      <mat-icon>forward_30</mat-icon>\r\n      <mat-icon>pause</mat-icon>\r\n      <mat-icon>play_arrow</mat-icon>\r\n      <mat-icon>playlist_play</mat-icon>\r\n      <mat-icon>replay_10</mat-icon>\r\n      <mat-icon>replay_30</mat-icon>\r\n      <mat-icon>skip_next</mat-icon>\r\n      <mat-icon>skip_previous</mat-icon>\r\n      <mat-icon>slow_motion_video</mat-icon>\r\n      <mat-icon>subtitles</mat-icon>\r\n      <mat-icon>volume_off</mat-icon>\r\n      <mat-icon>volume_mute</mat-icon>\r\n      <mat-icon>volume_down</mat-icon>\r\n      <mat-icon>volume_up</mat-icon>\r\n      <mat-icon>create</mat-icon>\r\n      <mat-icon>ondemand_video</mat-icon>\r\n    </mat-card><br />\r\n    <mat-card>\r\n      <mat-icon>send</mat-icon>\r\n      <mat-icon>battery_alert</mat-icon>\r\n      <mat-icon>battery_std</mat-icon>\r\n      <mat-icon>wallpaper</mat-icon>\r\n      <mat-icon>widgets</mat-icon>\r\n      <mat-icon>format_color_fill</mat-icon>\r\n      <mat-icon>format_color_text</mat-icon>\r\n      <mat-icon>format_align_center</mat-icon>\r\n      <mat-icon>format_align_justify</mat-icon>\r\n      <mat-icon>format_align_left</mat-icon>\r\n      <mat-icon>format_align_right</mat-icon>\r\n      <mat-icon>insert_chart</mat-icon>\r\n      <mat-icon>insert_photo</mat-icon>\r\n      <mat-icon>insert_comment</mat-icon>\r\n      <mat-icon>functions</mat-icon>\r\n      <mat-icon>more_vert</mat-icon>\r\n      <mat-icon>unfold_less</mat-icon>\r\n      <mat-icon>unfold_more</mat-icon>\r\n      <mat-icon>star</mat-icon>\r\n      <mat-icon>star_border</mat-icon>\r\n    </mat-card><br />\r\n  </mat-tab>\r\n</mat-tab-group>\r\n<mat-menu #userMenu=\"matMenu\">\r\n  <button mat-menu-item>Profile</button>\r\n  <button mat-menu-item>Theme</button>\r\n  <button mat-menu-item>Logoff</button>\r\n</mat-menu>\r\n<p>Module!</p>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/layout/layout.component.html":
/*!**********************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/layout/layout.component.html ***!
  \**********************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>layout works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/tab-list/tab-list.component.html":
/*!**************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/tab-list/tab-list.component.html ***!
  \**************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>tab-list works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/tab/tab.component.html":
/*!****************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/tab/tab.component.html ***!
  \****************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>tab works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-history-list/value-history-list.component.html":
/*!*******************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-history-list/value-history-list.component.html ***!
  \*******************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Value History</mat-card-title>\r\n    <mat-card-subtitle>Value History display</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"ValueHistorys\">\r\n      <ng-container matColumnDef=\"Timestamp\">\r\n        <mat-header-cell *matHeaderCellDef>Timestamp</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let record\">{{record.Timestamp}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Message\">\r\n        <mat-header-cell *matHeaderCellDef>History</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let record\">{{record.Message}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-history/value-history.component.html":
/*!*********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-history/value-history.component.html ***!
  \*********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>value-history works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-log-list/value-log-list.component.html":
/*!***********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-log-list/value-log-list.component.html ***!
  \***********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Value Log</mat-card-title>\r\n    <mat-card-subtitle>Value Log display</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"ValueLogs\">\r\n      <ng-container matColumnDef=\"Timestamp\">\r\n        <mat-header-cell *matHeaderCellDef>Timestamp</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let record\">{{record.Timestamp}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Message\">\r\n        <mat-header-cell *matHeaderCellDef>Log Message</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let record\">{{record.Message}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-log/value-log.component.html":
/*!*************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-log/value-log.component.html ***!
  \*************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>value-log works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value/value.component.html":
/*!*****************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value/value.component.html ***!
  \*****************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>value works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/device-list/device-list.component.html":
/*!*******************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/device-list/device-list.component.html ***!
  \*******************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Devices</mat-card-title>\r\n    <mat-card-subtitle>Device maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Devices/{{selectedRowNumber}}/Values\" matTooltip=\"Related Values\" [matTooltipShowDelay]=\"500\"><mat-icon>pageview</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Devices/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Devices/{{selectedRowNumber}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Devices/{{selectedRowNumber}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Devices\">\r\n      <ng-container matColumnDef=\"DeviceID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let device\">{{device.DeviceID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"InterfaceID\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Interface ID</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let device\" style=\"justify-content:center;\">{{device.InterfaceID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let device\">{{device.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"ExternalID\">\r\n        <mat-header-cell *matHeaderCellDef>External ID</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let device\">{{device.ExternalID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Timestamp\">\r\n        <mat-header-cell *matHeaderCellDef>Timestamp</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let device\">{{device.Timestamp}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.DeviceID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/device/device.component.html":
/*!*********************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/device/device.component.html ***!
  \*********************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>device works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-list/interface-list.component.html":
/*!*************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-list/interface-list.component.html ***!
  \*************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Interfaces</mat-card-title>\r\n    <mat-card-subtitle>Interface maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Interfaces/{{selectedRowNumber}}/InterfaceLogs\" matTooltip=\"Log\" [matTooltipShowDelay]=\"500\"><mat-icon>message</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Interfaces/{{selectedRowNumber}}/Devices\" matTooltip=\"Related Devices\" [matTooltipShowDelay]=\"500\"><mat-icon>extension</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Interfaces/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Interfaces/{{selectedRowNumber}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Interfaces/{{selectedRowNumber}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Interfaces\">\r\n      <ng-container matColumnDef=\"InterfaceID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let interface\">{{interface.InterfaceID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let interface\">{{interface.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Parameters\">\r\n        <mat-header-cell *matHeaderCellDef>Parameters</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let interface\">{{interface.Parameters}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Configuration\">\r\n        <mat-header-cell *matHeaderCellDef>Configuration</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let interface\">{{interface.Configuration}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Notifiable\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Notifiable</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let interface\" style=\"justify-content:center;\"><mat-icon>{interface.Notifiable, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Active\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Active</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let interface\" style=\"justify-content:center;\"><mat-icon>{interface.Active, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.InterfaceID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-log-list/interface-log-list.component.html":
/*!*********************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-log-list/interface-log-list.component.html ***!
  \*********************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Interface Log</mat-card-title>\r\n    <mat-card-subtitle>Interface Log display</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"InterfaceLogs\">\r\n      <ng-container matColumnDef=\"Timestamp\">\r\n        <mat-header-cell *matHeaderCellDef>Timestamp</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let log\">{{log.Timestamp}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Message\">\r\n        <mat-header-cell *matHeaderCellDef>Log Message</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let log\">{{log.Message}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-log/interface-log.component.html":
/*!***********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-log/interface-log.component.html ***!
  \***********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>interface-log works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface/interface.component.html":
/*!***************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface/interface.component.html ***!
  \***************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>interface works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-list/value-list.component.html":
/*!*****************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-list/value-list.component.html ***!
  \*****************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Values</mat-card-title>\r\n    <mat-card-subtitle>Value maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Values/{{selectedRowNumber}}/ValueLogs\" matTooltip=\"Log\" [matTooltipShowDelay]=\"500\"><mat-icon>message</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Values/{{selectedRowNumber}}/ValueHistorys\" matTooltip=\"History\" [matTooltipShowDelay]=\"500\"><mat-icon>history</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Values/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Values/{{selectedRowNumber}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Values/{{selectedRowNumber}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Values\">\r\n      <ng-container matColumnDef=\"ValueID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\">{{value.ValueID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\">{{value.Name}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"DeviceID\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Device ID</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\" style=\"justify-content:center;\">{{value.DeviceID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"UnitID\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Unit ID</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\" style=\"justify-content:center;\">{{value.UnitID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Value\">\r\n        <mat-header-cell *matHeaderCellDef>Value</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\">{{value.Value}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"RetensionDays\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Retension Days</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\" style=\"justify-content:center;\">{{value.RetensionDays}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"RetensionInterval\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Retension Interval</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\" style=\"justify-content:center;\">{{value.RetensionInterval}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Timestamp\">\r\n        <mat-header-cell *matHeaderCellDef>Timestamp</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let value\">{{value.Timestamp}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.ValueID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-script-list/value-script-list.component.html":
/*!*******************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-script-list/value-script-list.component.html ***!
  \*******************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>value-script-list works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-script/value-script.component.html":
/*!*********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-script/value-script.component.html ***!
  \*********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>value-script works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/role-list/role-list.component.html":
/*!***************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/role-list/role-list.component.html ***!
  \***************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Roles</mat-card-title>\r\n    <mat-card-subtitle>Role maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Roles/{{selectedRowNumber}}/Users\" matTooltip=\"Users in role\" [matTooltipShowDelay]=\"500\"><mat-icon>people</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Roles/{{selectedRowNumber}}/TableAccesss\" matTooltip=\"Role table access\" [matTooltipShowDelay]=\"500\"><mat-icon>pageview</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Roles/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Roles/{{selectedRowNumber}}\" matTooltip=\"Edit role\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Roles/{{selectedRowNumber}}\" matTooltip=\"Delete role\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Roles\">\r\n      <ng-container matColumnDef=\"RoleID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let role\">{{role.RoleID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let role\">{{role.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"RemoteAccess\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Remote Access</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let role\" style=\"justify-content:center;\"><mat-icon i18n>{role.RemoteAccess, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.RoleID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/role/role.component.html":
/*!*****************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/role/role.component.html ***!
  \*****************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>role works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/table-access-list/table-access-list.component.html":
/*!*******************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/table-access-list/table-access-list.component.html ***!
  \*******************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Table Access</mat-card-title>\r\n    <mat-card-subtitle>Table Access maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/TableAccesss/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/TableAccesss/{{selectedRowNumber}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/TableAccesss/{{selectedRowNumber}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"TableAccess\">\r\n      <ng-container matColumnDef=\"TableAccessID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\">{{tableaccess.TableAccessID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\">{{tableaccess.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"RoleID\">\r\n        <mat-header-cell *matHeaderCellDef>RoleID</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\">{{tableaccess.RoleID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"CanGET\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Can GET</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\" style=\"justify-content:center;\"><mat-icon>{tableaccess.CanGET, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"CanPOST\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Can POST</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\" style=\"justify-content:center;\"><mat-icon>{tableaccess.CanPOST, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"CanPUT\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Can PUT</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\" style=\"justify-content:center;\"><mat-icon>{tableaccess.CanPUT, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"CanPATCH\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Can PATCH</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\" style=\"justify-content:center;\"><mat-icon>{tableaccess.CanPATCH, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"CanDELETE\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Can DELETE</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\" style=\"justify-content:center;\"><mat-icon>{tableaccess.CanDELETE, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"PUTFields\">\r\n        <mat-header-cell *matHeaderCellDef>PUT Fields</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\">{{tableaccess.PUTFields}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"PATCHFields\">\r\n        <mat-header-cell *matHeaderCellDef>PATCH Fields</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let tableaccess\">{{tableaccess.PATCHFields}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.TableAccessID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/table-access/table-access.component.html":
/*!*********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/table-access/table-access.component.html ***!
  \*********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>table-access works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-list/user-list.component.html":
/*!***************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-list/user-list.component.html ***!
  \***************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Users</mat-card-title>\r\n    <mat-card-subtitle>User maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Users/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Users/{{selectedRowNumber}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Users/{{selectedRowNumber}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Users\">\r\n      <ng-container matColumnDef=\"UserID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.UserID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"UserName\">\r\n        <mat-header-cell *matHeaderCellDef>UserName</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.UserName}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Password\">\r\n        <mat-header-cell *matHeaderCellDef>Password</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.Password}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"FirstName\">\r\n        <mat-header-cell *matHeaderCellDef>First Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.FirstName}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"LastName\">\r\n        <mat-header-cell *matHeaderCellDef>Last Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.LastName}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"RoleID\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Role ID</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\" style=\"justify-content:center;\">{{user.RoleID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Active\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Active</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\" style=\"justify-content:center;\"><mat-icon>{user.Active, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"ForceChange\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Force Change</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\" style=\"justify-content:center;\"><mat-icon>{user.ForceChange, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"FailedAttempts\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Failed Attempts</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\" style=\"justify-content:center;\">{{user.FailedAttempts}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Theme\">\r\n        <mat-header-cell *matHeaderCellDef>Theme</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.Theme}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Timestamp\">\r\n        <mat-header-cell *matHeaderCellDef>Timestamp</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let user\">{{user.Timestamp}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.UserID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-session-list/user-session-list.component.html":
/*!*******************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-session-list/user-session-list.component.html ***!
  \*******************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>user-session-list works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-session/user-session.component.html":
/*!*********************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-session/user-session.component.html ***!
  \*********************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>user-session works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user/user.component.html":
/*!*****************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user/user.component.html ***!
  \*****************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>user works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/preference-list/preference-list.component.html":
/*!*************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/preference-list/preference-list.component.html ***!
  \*************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Preferences</mat-card-title>\r\n    <mat-card-subtitle>Preference maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Preferences/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Preferences/{{selectedRowNumber}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Preferences/{{selectedRowNumber}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n    <mat-card-content>\r\n      <mat-table [dataSource]=\"Preferences\">\r\n        <ng-container matColumnDef=\"PreferenceID\">\r\n          <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n          <mat-cell *matCellDef=\"let preference\">{{preference.PreferenceID}}</mat-cell>\r\n        </ng-container>\r\n\r\n        <ng-container matColumnDef=\"Name\">\r\n          <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n          <mat-cell *matCellDef=\"let preference\">{{preference.Name}}</mat-cell>\r\n\r\n        </ng-container>\r\n\r\n        <ng-container matColumnDef=\"Value\">\r\n          <mat-header-cell *matHeaderCellDef>Value</mat-header-cell>\r\n          <mat-cell *matCellDef=\"let preference\">{{preference.Value}}</mat-cell>\r\n        </ng-container>\r\n\r\n        <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n        <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.PreferenceID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n        </mat-row>\r\n      </mat-table>\r\n    </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/preference/preference.component.html":
/*!***************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/preference/preference.component.html ***!
  \***************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>preference works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-list/scene-list.component.html":
/*!***************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-list/scene-list.component.html ***!
  \***************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Scenes</mat-card-title>\r\n    <mat-card-subtitle>Scene maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Scenes/{{selectedRowNumber}}/ValueTimer\" matTooltip=\"Related Timer Values\" [matTooltipShowDelay]=\"500\"><mat-icon>pageview</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Scenes/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Scenes/{{selectedRow}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Scenes/{{selectedRow}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Scenes\">\r\n      <ng-container matColumnDef=\"SceneID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let scene\">{{scene.SceneID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let scene\">{{scene.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Active\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Active</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let scene\" style=\"justify-content:center;\"><mat-icon>{scene.Active, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.SceneID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-value-list/scene-value-list.component.html":
/*!***************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-value-list/scene-value-list.component.html ***!
  \***************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>scene-value-list works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-value/scene-value.component.html":
/*!*****************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-value/scene-value.component.html ***!
  \*****************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>scene-value works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene/scene.component.html":
/*!*****************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene/scene.component.html ***!
  \*****************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>scene works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/standard-script-list/standard-script-list.component.html":
/*!***********************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/standard-script-list/standard-script-list.component.html ***!
  \***********************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>standard-script-list works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/standard-script/standard-script.component.html":
/*!*************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/standard-script/standard-script.component.html ***!
  \*************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>standard-script works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/timer-plan-list/timer-plan-list.component.html":
/*!*************************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/timer-plan-list/timer-plan-list.component.html ***!
  \*************************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Timer Plans</mat-card-title>\r\n    <mat-card-subtitle>Timer Plan maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/TimerPlans/{{selectedRowNumber}}/ValueTimer\" matTooltip=\"Related Scene details\" [matTooltipShowDelay]=\"500\"><mat-icon>pageview</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/TimerPlans/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/TimerPlans/{{selectedRow}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/TimerPlans/{{selectedRow}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"TimerPlans\">\r\n      <ng-container matColumnDef=\"TimerPlanID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let timer\">{{timer.TimerPlanID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let timer\">{{timer.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Active\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Active</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let timer\" style=\"justify-content:center;\"><mat-icon>{timer.Active, plural, =1 {check_circle} other {highlight_off}}</mat-icon></mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.TimerPlanID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/timer-plan/timer-plan.component.html":
/*!***************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/timer-plan/timer-plan.component.html ***!
  \***************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>timer-plan works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/unit-list/unit-list.component.html":
/*!*************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/unit-list/unit-list.component.html ***!
  \*************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<mat-card>\r\n  <mat-card-header>\r\n    <img mat-card-avatar src=\"https://source.unsplash.com/random/200x200\">\r\n    <mat-card-title>Units</mat-card-title>\r\n    <mat-card-subtitle>Unit maintenance</mat-card-subtitle>\r\n    <div align=\"right\" style=\"width: 100%; margin: auto;\">\r\n      <button mat-mini-fab color=\"primary\" (click)=\"goBack();\" matTooltip=\"Previous\" [matTooltipShowDelay]=\"500\"><mat-icon>arrow_back</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" routerLink=\"/Units/{{selectedRowNumber}}\" matTooltip=\"New\" [matTooltipShowDelay]=\"500\"><mat-icon>add</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Units/{{selectedRow}}\" matTooltip=\"Edit\" [matTooltipShowDelay]=\"500\"><mat-icon>edit</mat-icon></button>\r\n      <button mat-mini-fab color=\"primary\" [disabled]=\"(selectedRow == 0)\" routerLink=\"/Units/{{selectedRow}}\" matTooltip=\"Delete\" [matTooltipShowDelay]=\"500\"><mat-icon>delete</mat-icon></button>\r\n    </div>\r\n  </mat-card-header>\r\n  <mat-card-content>\r\n    <mat-table [dataSource]=\"Units\">\r\n      <ng-container matColumnDef=\"UnitID\">\r\n        <mat-header-cell *matHeaderCellDef>#</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let unit\">{{unit.UnitID}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Name\">\r\n        <mat-header-cell *matHeaderCellDef>Name</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let unit\">{{unit.Name}}</mat-cell>\r\n\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Minimum\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Minimum</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let unit\" style=\"justify-content:center;\">{{unit.Minimum}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"Maximum\">\r\n        <mat-header-cell *matHeaderCellDef style=\"justify-content:center;\">Maximum</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let unit\" style=\"justify-content:center;\">{{unit.Maximum}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"IconList\">\r\n        <mat-header-cell *matHeaderCellDef>Icon List</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let unit\">{{unit.IconList}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <ng-container matColumnDef=\"TextLabels\">\r\n        <mat-header-cell *matHeaderCellDef>Text Labels</mat-header-cell>\r\n        <mat-cell *matCellDef=\"let unit\">{{unit.TextLabels}}</mat-cell>\r\n      </ng-container>\r\n\r\n      <mat-header-row *matHeaderRowDef=\"tableColumns\"></mat-header-row>\r\n\r\n      <mat-row *matRowDef=\"let row; columns: tableColumns; \"\r\n               (click)=\"(selectedRow == row) ? selectedRow = 0 : selectedRow = row; selectedRowNumber = row.UnitID;\"\r\n               [style.background]=\"selectedRow == row ? 'lightblue' : ''\">\r\n      </mat-row>\r\n    </mat-table>\r\n  </mat-card-content>\r\n</mat-card>\r\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/unit/unit.component.html":
/*!***************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/unit/unit.component.html ***!
  \***************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>unit works!</p>\n");

/***/ }),

/***/ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/value-timer/value-timer.component.html":
/*!*****************************************************************************************************!*\
  !*** ./node_modules/raw-loader/dist/cjs.js!./src/app/system/value-timer/value-timer.component.html ***!
  \*****************************************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("<p>value-timer works!</p>\n");

/***/ }),

/***/ "./node_modules/tslib/tslib.es6.js":
/*!*****************************************!*\
  !*** ./node_modules/tslib/tslib.es6.js ***!
  \*****************************************/
/*! exports provided: __extends, __assign, __rest, __decorate, __param, __metadata, __awaiter, __generator, __exportStar, __values, __read, __spread, __spreadArrays, __await, __asyncGenerator, __asyncDelegator, __asyncValues, __makeTemplateObject, __importStar, __importDefault */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__extends", function() { return __extends; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__assign", function() { return __assign; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__rest", function() { return __rest; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__decorate", function() { return __decorate; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__param", function() { return __param; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__metadata", function() { return __metadata; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__awaiter", function() { return __awaiter; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__generator", function() { return __generator; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__exportStar", function() { return __exportStar; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__values", function() { return __values; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__read", function() { return __read; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__spread", function() { return __spread; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__spreadArrays", function() { return __spreadArrays; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__await", function() { return __await; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__asyncGenerator", function() { return __asyncGenerator; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__asyncDelegator", function() { return __asyncDelegator; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__asyncValues", function() { return __asyncValues; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__makeTemplateObject", function() { return __makeTemplateObject; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__importStar", function() { return __importStar; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "__importDefault", function() { return __importDefault; });
/*! *****************************************************************************
Copyright (c) Microsoft Corporation. All rights reserved.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at http://www.apache.org/licenses/LICENSE-2.0

THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
MERCHANTABLITY OR NON-INFRINGEMENT.

See the Apache Version 2.0 License for specific language governing permissions
and limitations under the License.
***************************************************************************** */
/* global Reflect, Promise */

var extendStatics = function(d, b) {
    extendStatics = Object.setPrototypeOf ||
        ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
        function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
    return extendStatics(d, b);
};

function __extends(d, b) {
    extendStatics(d, b);
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
}

var __assign = function() {
    __assign = Object.assign || function __assign(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p)) t[p] = s[p];
        }
        return t;
    }
    return __assign.apply(this, arguments);
}

function __rest(s, e) {
    var t = {};
    for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
        t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
        for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
            if (e.indexOf(p[i]) < 0 && Object.prototype.propertyIsEnumerable.call(s, p[i]))
                t[p[i]] = s[p[i]];
        }
    return t;
}

function __decorate(decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
}

function __param(paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); }
}

function __metadata(metadataKey, metadataValue) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(metadataKey, metadataValue);
}

function __awaiter(thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
}

function __generator(thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
}

function __exportStar(m, exports) {
    for (var p in m) if (!exports.hasOwnProperty(p)) exports[p] = m[p];
}

function __values(o) {
    var m = typeof Symbol === "function" && o[Symbol.iterator], i = 0;
    if (m) return m.call(o);
    return {
        next: function () {
            if (o && i >= o.length) o = void 0;
            return { value: o && o[i++], done: !o };
        }
    };
}

function __read(o, n) {
    var m = typeof Symbol === "function" && o[Symbol.iterator];
    if (!m) return o;
    var i = m.call(o), r, ar = [], e;
    try {
        while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
    }
    catch (error) { e = { error: error }; }
    finally {
        try {
            if (r && !r.done && (m = i["return"])) m.call(i);
        }
        finally { if (e) throw e.error; }
    }
    return ar;
}

function __spread() {
    for (var ar = [], i = 0; i < arguments.length; i++)
        ar = ar.concat(__read(arguments[i]));
    return ar;
}

function __spreadArrays() {
    for (var s = 0, i = 0, il = arguments.length; i < il; i++) s += arguments[i].length;
    for (var r = Array(s), k = 0, i = 0; i < il; i++)
        for (var a = arguments[i], j = 0, jl = a.length; j < jl; j++, k++)
            r[k] = a[j];
    return r;
};

function __await(v) {
    return this instanceof __await ? (this.v = v, this) : new __await(v);
}

function __asyncGenerator(thisArg, _arguments, generator) {
    if (!Symbol.asyncIterator) throw new TypeError("Symbol.asyncIterator is not defined.");
    var g = generator.apply(thisArg, _arguments || []), i, q = [];
    return i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i;
    function verb(n) { if (g[n]) i[n] = function (v) { return new Promise(function (a, b) { q.push([n, v, a, b]) > 1 || resume(n, v); }); }; }
    function resume(n, v) { try { step(g[n](v)); } catch (e) { settle(q[0][3], e); } }
    function step(r) { r.value instanceof __await ? Promise.resolve(r.value.v).then(fulfill, reject) : settle(q[0][2], r); }
    function fulfill(value) { resume("next", value); }
    function reject(value) { resume("throw", value); }
    function settle(f, v) { if (f(v), q.shift(), q.length) resume(q[0][0], q[0][1]); }
}

function __asyncDelegator(o) {
    var i, p;
    return i = {}, verb("next"), verb("throw", function (e) { throw e; }), verb("return"), i[Symbol.iterator] = function () { return this; }, i;
    function verb(n, f) { i[n] = o[n] ? function (v) { return (p = !p) ? { value: __await(o[n](v)), done: n === "return" } : f ? f(v) : v; } : f; }
}

function __asyncValues(o) {
    if (!Symbol.asyncIterator) throw new TypeError("Symbol.asyncIterator is not defined.");
    var m = o[Symbol.asyncIterator], i;
    return m ? m.call(o) : (o = typeof __values === "function" ? __values(o) : o[Symbol.iterator](), i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i);
    function verb(n) { i[n] = o[n] && function (v) { return new Promise(function (resolve, reject) { v = o[n](v), settle(resolve, reject, v.done, v.value); }); }; }
    function settle(resolve, reject, d, v) { Promise.resolve(v).then(function(v) { resolve({ value: v, done: d }); }, reject); }
}

function __makeTemplateObject(cooked, raw) {
    if (Object.defineProperty) { Object.defineProperty(cooked, "raw", { value: raw }); } else { cooked.raw = raw; }
    return cooked;
};

function __importStar(mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result.default = mod;
    return result;
}

function __importDefault(mod) {
    return (mod && mod.__esModule) ? mod : { default: mod };
}


/***/ }),

/***/ "./src/app/app-routing.module.ts":
/*!***************************************!*\
  !*** ./src/app/app-routing.module.ts ***!
  \***************************************/
/*! exports provided: AppRoutingModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "AppRoutingModule", function() { return AppRoutingModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");



const routes = [
    { path: '', redirectTo: '/Layouts', pathMatch: 'full' }
];
let AppRoutingModule = class AppRoutingModule {
};
AppRoutingModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        imports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"].forRoot(routes)],
        exports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"]]
    })
], AppRoutingModule);



/***/ }),

/***/ "./src/app/app.component.css":
/*!***********************************!*\
  !*** ./src/app/app.component.css ***!
  \***********************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (".root-container {\r\n  width: 100vw;\r\n  height: 100vh;\r\n  display: grid;\r\n  grid-template-columns: auto;\r\n  grid-template-rows: 0.5fr auto;\r\n  position: relative;\r\n}\r\n\r\n.main-toolbar {\r\n  width: 100%;\r\n  border-radius: 4px;\r\n  display: grid;\r\n  grid-template-columns: 32px auto 24px;\r\n  grid-template-rows: 1fr;\r\n  box-shadow: 0 0 2rem rgba(0, 0, 255, 0.1);\r\n  height: 4rem;\r\n  margin: 0;\r\n  justify-items: center;\r\n  transition: background-color 500ms linear;\r\n  -webkit-animation: 1s ease-in-out 0ms 1 fadein;\r\n          animation: 1s ease-in-out 0ms 1 fadein;\r\n}\r\n\r\nmat-drawer {\r\n  height: 100vh;\r\n  padding: 12px;\r\n  display: flex;\r\n  flex-direction: column;\r\n  align-items: flex-start;\r\n}\r\n\r\nmat-drawer-content {\r\n  height: 100vh;\r\n  padding: 12px;\r\n  display: flex;\r\n  flex-direction: column;\r\n  align-items: flex-start;\r\n}\r\n\r\n/* Menu items that are routed */\r\n\r\n.active {\r\n  font-weight: bold;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvYXBwLmNvbXBvbmVudC5jc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUE7RUFDRSxZQUFZO0VBQ1osYUFBYTtFQUNiLGFBQWE7RUFDYiwyQkFBMkI7RUFDM0IsOEJBQThCO0VBQzlCLGtCQUFrQjtBQUNwQjs7QUFFQTtFQUNFLFdBQVc7RUFDWCxrQkFBa0I7RUFDbEIsYUFBYTtFQUNiLHFDQUFxQztFQUNyQyx1QkFBdUI7RUFDdkIseUNBQXlDO0VBQ3pDLFlBQVk7RUFDWixTQUFTO0VBQ1QscUJBQXFCO0VBQ3JCLHlDQUF5QztFQUN6Qyw4Q0FBc0M7VUFBdEMsc0NBQXNDO0FBQ3hDOztBQUVBO0VBQ0UsYUFBYTtFQUNiLGFBQWE7RUFDYixhQUFhO0VBQ2Isc0JBQXNCO0VBQ3RCLHVCQUF1QjtBQUN6Qjs7QUFFQTtFQUNFLGFBQWE7RUFDYixhQUFhO0VBQ2IsYUFBYTtFQUNiLHNCQUFzQjtFQUN0Qix1QkFBdUI7QUFDekI7O0FBRUEsK0JBQStCOztBQUMvQjtFQUNFLGlCQUFpQjtBQUNuQiIsImZpbGUiOiJzcmMvYXBwL2FwcC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiLnJvb3QtY29udGFpbmVyIHtcclxuICB3aWR0aDogMTAwdnc7XHJcbiAgaGVpZ2h0OiAxMDB2aDtcclxuICBkaXNwbGF5OiBncmlkO1xyXG4gIGdyaWQtdGVtcGxhdGUtY29sdW1uczogYXV0bztcclxuICBncmlkLXRlbXBsYXRlLXJvd3M6IDAuNWZyIGF1dG87XHJcbiAgcG9zaXRpb246IHJlbGF0aXZlO1xyXG59XHJcblxyXG4ubWFpbi10b29sYmFyIHtcclxuICB3aWR0aDogMTAwJTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbiAgZGlzcGxheTogZ3JpZDtcclxuICBncmlkLXRlbXBsYXRlLWNvbHVtbnM6IDMycHggYXV0byAyNHB4O1xyXG4gIGdyaWQtdGVtcGxhdGUtcm93czogMWZyO1xyXG4gIGJveC1zaGFkb3c6IDAgMCAycmVtIHJnYmEoMCwgMCwgMjU1LCAwLjEpO1xyXG4gIGhlaWdodDogNHJlbTtcclxuICBtYXJnaW46IDA7XHJcbiAganVzdGlmeS1pdGVtczogY2VudGVyO1xyXG4gIHRyYW5zaXRpb246IGJhY2tncm91bmQtY29sb3IgNTAwbXMgbGluZWFyO1xyXG4gIGFuaW1hdGlvbjogMXMgZWFzZS1pbi1vdXQgMG1zIDEgZmFkZWluO1xyXG59XHJcblxyXG5tYXQtZHJhd2VyIHtcclxuICBoZWlnaHQ6IDEwMHZoO1xyXG4gIHBhZGRpbmc6IDEycHg7XHJcbiAgZGlzcGxheTogZmxleDtcclxuICBmbGV4LWRpcmVjdGlvbjogY29sdW1uO1xyXG4gIGFsaWduLWl0ZW1zOiBmbGV4LXN0YXJ0O1xyXG59XHJcblxyXG5tYXQtZHJhd2VyLWNvbnRlbnQge1xyXG4gIGhlaWdodDogMTAwdmg7XHJcbiAgcGFkZGluZzogMTJweDtcclxuICBkaXNwbGF5OiBmbGV4O1xyXG4gIGZsZXgtZGlyZWN0aW9uOiBjb2x1bW47XHJcbiAgYWxpZ24taXRlbXM6IGZsZXgtc3RhcnQ7XHJcbn1cclxuXHJcbi8qIE1lbnUgaXRlbXMgdGhhdCBhcmUgcm91dGVkICovXHJcbi5hY3RpdmUge1xyXG4gIGZvbnQtd2VpZ2h0OiBib2xkO1xyXG59XHJcbiJdfQ== */");

/***/ }),

/***/ "./src/app/app.component.ts":
/*!**********************************!*\
  !*** ./src/app/app.component.ts ***!
  \**********************************/
/*! exports provided: AppComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "AppComponent", function() { return AppComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let AppComponent = class AppComponent {
    constructor() {
        this.title = 'Domoticz';
    }
};
AppComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-root',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./app.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/app.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./app.component.css */ "./src/app/app.component.css")).default]
    })
], AppComponent);



/***/ }),

/***/ "./src/app/app.module.ts":
/*!*******************************!*\
  !*** ./src/app/app.module.ts ***!
  \*******************************/
/*! exports provided: AppModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "AppModule", function() { return AppModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_forms__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/forms */ "./node_modules/@angular/forms/fesm2015/forms.js");
/* harmony import */ var _angular_platform_browser__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/platform-browser */ "./node_modules/@angular/platform-browser/fesm2015/platform-browser.js");
/* harmony import */ var _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/platform-browser/animations */ "./node_modules/@angular/platform-browser/fesm2015/animations.js");
/* harmony import */ var _angular_material_button__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! @angular/material/button */ "./node_modules/@angular/material/esm2015/button.js");
/* harmony import */ var _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! @angular/material/button-toggle */ "./node_modules/@angular/material/esm2015/button-toggle.js");
/* harmony import */ var _angular_material_card__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! @angular/material/card */ "./node_modules/@angular/material/esm2015/card.js");
/* harmony import */ var _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! @angular/material/checkbox */ "./node_modules/@angular/material/esm2015/checkbox.js");
/* harmony import */ var _angular_material_chips__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! @angular/material/chips */ "./node_modules/@angular/material/esm2015/chips.js");
/* harmony import */ var _angular_material_stepper__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! @angular/material/stepper */ "./node_modules/@angular/material/esm2015/stepper.js");
/* harmony import */ var _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_11__ = __webpack_require__(/*! @angular/material/datepicker */ "./node_modules/@angular/material/esm2015/datepicker.js");
/* harmony import */ var _angular_material_icon__WEBPACK_IMPORTED_MODULE_12__ = __webpack_require__(/*! @angular/material/icon */ "./node_modules/@angular/material/esm2015/icon.js");
/* harmony import */ var _angular_material_input__WEBPACK_IMPORTED_MODULE_13__ = __webpack_require__(/*! @angular/material/input */ "./node_modules/@angular/material/esm2015/input.js");
/* harmony import */ var _angular_material_list__WEBPACK_IMPORTED_MODULE_14__ = __webpack_require__(/*! @angular/material/list */ "./node_modules/@angular/material/esm2015/list.js");
/* harmony import */ var _angular_material_menu__WEBPACK_IMPORTED_MODULE_15__ = __webpack_require__(/*! @angular/material/menu */ "./node_modules/@angular/material/esm2015/menu.js");
/* harmony import */ var _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_16__ = __webpack_require__(/*! @angular/material/sidenav */ "./node_modules/@angular/material/esm2015/sidenav.js");
/* harmony import */ var _angular_material_slider__WEBPACK_IMPORTED_MODULE_17__ = __webpack_require__(/*! @angular/material/slider */ "./node_modules/@angular/material/esm2015/slider.js");
/* harmony import */ var _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_18__ = __webpack_require__(/*! @angular/material/slide-toggle */ "./node_modules/@angular/material/esm2015/slide-toggle.js");
/* harmony import */ var _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_19__ = __webpack_require__(/*! @angular/material/toolbar */ "./node_modules/@angular/material/esm2015/toolbar.js");
/* harmony import */ var _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_20__ = __webpack_require__(/*! @angular/material/tooltip */ "./node_modules/@angular/material/esm2015/tooltip.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_21__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _app_component__WEBPACK_IMPORTED_MODULE_22__ = __webpack_require__(/*! ./app.component */ "./src/app/app.component.ts");
/* harmony import */ var _app_routing_module__WEBPACK_IMPORTED_MODULE_23__ = __webpack_require__(/*! ./app-routing.module */ "./src/app/app-routing.module.ts");
/* harmony import */ var src_app_dashboard_dashboard_module__WEBPACK_IMPORTED_MODULE_24__ = __webpack_require__(/*! src/app/dashboard/dashboard.module */ "./src/app/dashboard/dashboard.module.ts");
/* harmony import */ var src_app_identity_identity_module__WEBPACK_IMPORTED_MODULE_25__ = __webpack_require__(/*! src/app/identity/identity.module */ "./src/app/identity/identity.module.ts");
/* harmony import */ var src_app_hardware_hardware_module__WEBPACK_IMPORTED_MODULE_26__ = __webpack_require__(/*! src/app/hardware/hardware.module */ "./src/app/hardware/hardware.module.ts");
/* harmony import */ var src_app_system_system_module__WEBPACK_IMPORTED_MODULE_27__ = __webpack_require__(/*! src/app/system/system.module */ "./src/app/system/system.module.ts");
/* harmony import */ var src_app_detail_detail_module__WEBPACK_IMPORTED_MODULE_28__ = __webpack_require__(/*! src/app/detail/detail.module */ "./src/app/detail/detail.module.ts");
























// Application Modules





let AppModule = class AppModule {
};
AppModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        declarations: [
            _app_component__WEBPACK_IMPORTED_MODULE_22__["AppComponent"]
        ],
        imports: [
            _app_routing_module__WEBPACK_IMPORTED_MODULE_23__["AppRoutingModule"],
            _angular_common_http__WEBPACK_IMPORTED_MODULE_21__["HttpClientModule"],
            _angular_platform_browser__WEBPACK_IMPORTED_MODULE_3__["BrowserModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_2__["FormsModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_2__["ReactiveFormsModule"],
            _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_4__["BrowserAnimationsModule"],
            _angular_material_button__WEBPACK_IMPORTED_MODULE_5__["MatButtonModule"],
            _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_6__["MatButtonToggleModule"],
            _angular_material_card__WEBPACK_IMPORTED_MODULE_7__["MatCardModule"],
            _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_8__["MatCheckboxModule"],
            _angular_material_chips__WEBPACK_IMPORTED_MODULE_9__["MatChipsModule"],
            _angular_material_stepper__WEBPACK_IMPORTED_MODULE_10__["MatStepperModule"],
            _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_11__["MatDatepickerModule"],
            _angular_material_icon__WEBPACK_IMPORTED_MODULE_12__["MatIconModule"],
            _angular_material_input__WEBPACK_IMPORTED_MODULE_13__["MatInputModule"],
            _angular_material_list__WEBPACK_IMPORTED_MODULE_14__["MatListModule"],
            _angular_material_menu__WEBPACK_IMPORTED_MODULE_15__["MatMenuModule"],
            _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_16__["MatSidenavModule"],
            _angular_material_slider__WEBPACK_IMPORTED_MODULE_17__["MatSliderModule"],
            _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_18__["MatSlideToggleModule"],
            _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_19__["MatToolbarModule"],
            _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_20__["MatTooltipModule"],
            src_app_dashboard_dashboard_module__WEBPACK_IMPORTED_MODULE_24__["DashboardModule"],
            src_app_identity_identity_module__WEBPACK_IMPORTED_MODULE_25__["IdentityModule"],
            src_app_hardware_hardware_module__WEBPACK_IMPORTED_MODULE_26__["HardwareModule"],
            src_app_system_system_module__WEBPACK_IMPORTED_MODULE_27__["SystemModule"],
            src_app_detail_detail_module__WEBPACK_IMPORTED_MODULE_28__["DetailModule"]
        ],
        providers: [],
        bootstrap: [_app_component__WEBPACK_IMPORTED_MODULE_22__["AppComponent"]]
    })
], AppModule);



/***/ }),

/***/ "./src/app/dashboard/dashboard-routing.module.ts":
/*!*******************************************************!*\
  !*** ./src/app/dashboard/dashboard-routing.module.ts ***!
  \*******************************************************/
/*! exports provided: DashboardRoutingModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DashboardRoutingModule", function() { return DashboardRoutingModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var src_app_dashboard_layout_layout_component__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! src/app/dashboard/layout/layout.component */ "./src/app/dashboard/layout/layout.component.ts");
/* harmony import */ var src_app_dashboard_layout_list_layout_list_component__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! src/app/dashboard/layout-list/layout-list.component */ "./src/app/dashboard/layout-list/layout-list.component.ts");





const dashboardRoutes = [
    { path: 'Layouts', component: src_app_dashboard_layout_list_layout_list_component__WEBPACK_IMPORTED_MODULE_4__["LayoutListComponent"] },
    { path: 'Layouts/:id', component: src_app_dashboard_layout_layout_component__WEBPACK_IMPORTED_MODULE_3__["LayoutComponent"] }
];
let DashboardRoutingModule = class DashboardRoutingModule {
};
DashboardRoutingModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        imports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"].forChild(dashboardRoutes)],
        exports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"]]
    })
], DashboardRoutingModule);



/***/ }),

/***/ "./src/app/dashboard/dashboard.module.ts":
/*!***********************************************!*\
  !*** ./src/app/dashboard/dashboard.module.ts ***!
  \***********************************************/
/*! exports provided: DashboardModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DashboardModule", function() { return DashboardModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_forms__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/forms */ "./node_modules/@angular/forms/fesm2015/forms.js");
/* harmony import */ var _angular_platform_browser__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/platform-browser */ "./node_modules/@angular/platform-browser/fesm2015/platform-browser.js");
/* harmony import */ var _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! @angular/platform-browser/animations */ "./node_modules/@angular/platform-browser/fesm2015/animations.js");
/* harmony import */ var _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! @angular/material/autocomplete */ "./node_modules/@angular/material/esm2015/autocomplete.js");
/* harmony import */ var _angular_material_badge__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! @angular/material/badge */ "./node_modules/@angular/material/esm2015/badge.js");
/* harmony import */ var _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! @angular/material/bottom-sheet */ "./node_modules/@angular/material/esm2015/bottom-sheet.js");
/* harmony import */ var _angular_material_button__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! @angular/material/button */ "./node_modules/@angular/material/esm2015/button.js");
/* harmony import */ var _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! @angular/material/button-toggle */ "./node_modules/@angular/material/esm2015/button-toggle.js");
/* harmony import */ var _angular_material_card__WEBPACK_IMPORTED_MODULE_11__ = __webpack_require__(/*! @angular/material/card */ "./node_modules/@angular/material/esm2015/card.js");
/* harmony import */ var _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_12__ = __webpack_require__(/*! @angular/material/checkbox */ "./node_modules/@angular/material/esm2015/checkbox.js");
/* harmony import */ var _angular_material_chips__WEBPACK_IMPORTED_MODULE_13__ = __webpack_require__(/*! @angular/material/chips */ "./node_modules/@angular/material/esm2015/chips.js");
/* harmony import */ var _angular_material_stepper__WEBPACK_IMPORTED_MODULE_14__ = __webpack_require__(/*! @angular/material/stepper */ "./node_modules/@angular/material/esm2015/stepper.js");
/* harmony import */ var _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_15__ = __webpack_require__(/*! @angular/material/datepicker */ "./node_modules/@angular/material/esm2015/datepicker.js");
/* harmony import */ var _angular_material_dialog__WEBPACK_IMPORTED_MODULE_16__ = __webpack_require__(/*! @angular/material/dialog */ "./node_modules/@angular/material/esm2015/dialog.js");
/* harmony import */ var _angular_material_divider__WEBPACK_IMPORTED_MODULE_17__ = __webpack_require__(/*! @angular/material/divider */ "./node_modules/@angular/material/esm2015/divider.js");
/* harmony import */ var _angular_material_expansion__WEBPACK_IMPORTED_MODULE_18__ = __webpack_require__(/*! @angular/material/expansion */ "./node_modules/@angular/material/esm2015/expansion.js");
/* harmony import */ var _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_19__ = __webpack_require__(/*! @angular/material/grid-list */ "./node_modules/@angular/material/esm2015/grid-list.js");
/* harmony import */ var _angular_material_icon__WEBPACK_IMPORTED_MODULE_20__ = __webpack_require__(/*! @angular/material/icon */ "./node_modules/@angular/material/esm2015/icon.js");
/* harmony import */ var _angular_material_input__WEBPACK_IMPORTED_MODULE_21__ = __webpack_require__(/*! @angular/material/input */ "./node_modules/@angular/material/esm2015/input.js");
/* harmony import */ var _angular_material_list__WEBPACK_IMPORTED_MODULE_22__ = __webpack_require__(/*! @angular/material/list */ "./node_modules/@angular/material/esm2015/list.js");
/* harmony import */ var _angular_material_menu__WEBPACK_IMPORTED_MODULE_23__ = __webpack_require__(/*! @angular/material/menu */ "./node_modules/@angular/material/esm2015/menu.js");
/* harmony import */ var _angular_material_core__WEBPACK_IMPORTED_MODULE_24__ = __webpack_require__(/*! @angular/material/core */ "./node_modules/@angular/material/esm2015/core.js");
/* harmony import */ var _angular_material_paginator__WEBPACK_IMPORTED_MODULE_25__ = __webpack_require__(/*! @angular/material/paginator */ "./node_modules/@angular/material/esm2015/paginator.js");
/* harmony import */ var _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_26__ = __webpack_require__(/*! @angular/material/progress-bar */ "./node_modules/@angular/material/esm2015/progress-bar.js");
/* harmony import */ var _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_27__ = __webpack_require__(/*! @angular/material/progress-spinner */ "./node_modules/@angular/material/esm2015/progress-spinner.js");
/* harmony import */ var _angular_material_radio__WEBPACK_IMPORTED_MODULE_28__ = __webpack_require__(/*! @angular/material/radio */ "./node_modules/@angular/material/esm2015/radio.js");
/* harmony import */ var _angular_material_select__WEBPACK_IMPORTED_MODULE_29__ = __webpack_require__(/*! @angular/material/select */ "./node_modules/@angular/material/esm2015/select.js");
/* harmony import */ var _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_30__ = __webpack_require__(/*! @angular/material/sidenav */ "./node_modules/@angular/material/esm2015/sidenav.js");
/* harmony import */ var _angular_material_slider__WEBPACK_IMPORTED_MODULE_31__ = __webpack_require__(/*! @angular/material/slider */ "./node_modules/@angular/material/esm2015/slider.js");
/* harmony import */ var _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_32__ = __webpack_require__(/*! @angular/material/slide-toggle */ "./node_modules/@angular/material/esm2015/slide-toggle.js");
/* harmony import */ var _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_33__ = __webpack_require__(/*! @angular/material/snack-bar */ "./node_modules/@angular/material/esm2015/snack-bar.js");
/* harmony import */ var _angular_material_sort__WEBPACK_IMPORTED_MODULE_34__ = __webpack_require__(/*! @angular/material/sort */ "./node_modules/@angular/material/esm2015/sort.js");
/* harmony import */ var _angular_material_table__WEBPACK_IMPORTED_MODULE_35__ = __webpack_require__(/*! @angular/material/table */ "./node_modules/@angular/material/esm2015/table.js");
/* harmony import */ var _angular_material_tabs__WEBPACK_IMPORTED_MODULE_36__ = __webpack_require__(/*! @angular/material/tabs */ "./node_modules/@angular/material/esm2015/tabs.js");
/* harmony import */ var _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_37__ = __webpack_require__(/*! @angular/material/toolbar */ "./node_modules/@angular/material/esm2015/toolbar.js");
/* harmony import */ var _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_38__ = __webpack_require__(/*! @angular/material/tooltip */ "./node_modules/@angular/material/esm2015/tooltip.js");
/* harmony import */ var _angular_material_tree__WEBPACK_IMPORTED_MODULE_39__ = __webpack_require__(/*! @angular/material/tree */ "./node_modules/@angular/material/esm2015/tree.js");
/* harmony import */ var src_app_dashboard_layout_layout_component__WEBPACK_IMPORTED_MODULE_40__ = __webpack_require__(/*! src/app/dashboard/layout/layout.component */ "./src/app/dashboard/layout/layout.component.ts");
/* harmony import */ var src_app_dashboard_layout_list_layout_list_component__WEBPACK_IMPORTED_MODULE_41__ = __webpack_require__(/*! src/app/dashboard/layout-list/layout-list.component */ "./src/app/dashboard/layout-list/layout-list.component.ts");
/* harmony import */ var src_app_dashboard_tab_tab_component__WEBPACK_IMPORTED_MODULE_42__ = __webpack_require__(/*! src/app/dashboard/tab/tab.component */ "./src/app/dashboard/tab/tab.component.ts");
/* harmony import */ var src_app_dashboard_tab_list_tab_list_component__WEBPACK_IMPORTED_MODULE_43__ = __webpack_require__(/*! src/app/dashboard/tab-list/tab-list.component */ "./src/app/dashboard/tab-list/tab-list.component.ts");
/* harmony import */ var src_app_dashboard_element_element_component__WEBPACK_IMPORTED_MODULE_44__ = __webpack_require__(/*! src/app/dashboard/element/element.component */ "./src/app/dashboard/element/element.component.ts");
/* harmony import */ var src_app_dashboard_element_list_element_list_component__WEBPACK_IMPORTED_MODULE_45__ = __webpack_require__(/*! src/app/dashboard/element-list/element-list.component */ "./src/app/dashboard/element-list/element-list.component.ts");
/* harmony import */ var src_app_dashboard_dashboard_routing_module__WEBPACK_IMPORTED_MODULE_46__ = __webpack_require__(/*! src/app/dashboard/dashboard-routing.module */ "./src/app/dashboard/dashboard-routing.module.ts");








































// Dashboard







let DashboardModule = class DashboardModule {
};
DashboardModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        declarations: [
            src_app_dashboard_layout_layout_component__WEBPACK_IMPORTED_MODULE_40__["LayoutComponent"],
            src_app_dashboard_layout_list_layout_list_component__WEBPACK_IMPORTED_MODULE_41__["LayoutListComponent"],
            src_app_dashboard_tab_tab_component__WEBPACK_IMPORTED_MODULE_42__["TabComponent"],
            src_app_dashboard_tab_list_tab_list_component__WEBPACK_IMPORTED_MODULE_43__["TabListComponent"],
            src_app_dashboard_element_element_component__WEBPACK_IMPORTED_MODULE_44__["ElementComponent"],
            src_app_dashboard_element_list_element_list_component__WEBPACK_IMPORTED_MODULE_45__["ElementListComponent"]
        ],
        imports: [
            _angular_common__WEBPACK_IMPORTED_MODULE_2__["CommonModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["FormsModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["ReactiveFormsModule"],
            _angular_platform_browser__WEBPACK_IMPORTED_MODULE_4__["BrowserModule"],
            _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_5__["BrowserAnimationsModule"],
            _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_6__["MatAutocompleteModule"],
            _angular_material_badge__WEBPACK_IMPORTED_MODULE_7__["MatBadgeModule"],
            _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_8__["MatBottomSheetModule"],
            _angular_material_button__WEBPACK_IMPORTED_MODULE_9__["MatButtonModule"],
            _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_10__["MatButtonToggleModule"],
            _angular_material_card__WEBPACK_IMPORTED_MODULE_11__["MatCardModule"],
            _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_12__["MatCheckboxModule"],
            _angular_material_chips__WEBPACK_IMPORTED_MODULE_13__["MatChipsModule"],
            _angular_material_stepper__WEBPACK_IMPORTED_MODULE_14__["MatStepperModule"],
            _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_15__["MatDatepickerModule"],
            _angular_material_dialog__WEBPACK_IMPORTED_MODULE_16__["MatDialogModule"],
            _angular_material_divider__WEBPACK_IMPORTED_MODULE_17__["MatDividerModule"],
            _angular_material_expansion__WEBPACK_IMPORTED_MODULE_18__["MatExpansionModule"],
            _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_19__["MatGridListModule"],
            _angular_material_icon__WEBPACK_IMPORTED_MODULE_20__["MatIconModule"],
            _angular_material_input__WEBPACK_IMPORTED_MODULE_21__["MatInputModule"],
            _angular_material_list__WEBPACK_IMPORTED_MODULE_22__["MatListModule"],
            _angular_material_menu__WEBPACK_IMPORTED_MODULE_23__["MatMenuModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_24__["MatNativeDateModule"],
            _angular_material_paginator__WEBPACK_IMPORTED_MODULE_25__["MatPaginatorModule"],
            _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_26__["MatProgressBarModule"],
            _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_27__["MatProgressSpinnerModule"],
            _angular_material_radio__WEBPACK_IMPORTED_MODULE_28__["MatRadioModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_24__["MatRippleModule"],
            _angular_material_select__WEBPACK_IMPORTED_MODULE_29__["MatSelectModule"],
            _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_30__["MatSidenavModule"],
            _angular_material_slider__WEBPACK_IMPORTED_MODULE_31__["MatSliderModule"],
            _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_32__["MatSlideToggleModule"],
            _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_33__["MatSnackBarModule"],
            _angular_material_sort__WEBPACK_IMPORTED_MODULE_34__["MatSortModule"],
            _angular_material_table__WEBPACK_IMPORTED_MODULE_35__["MatTableModule"],
            _angular_material_tabs__WEBPACK_IMPORTED_MODULE_36__["MatTabsModule"],
            _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_37__["MatToolbarModule"],
            _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_38__["MatTooltipModule"],
            _angular_material_tree__WEBPACK_IMPORTED_MODULE_39__["MatTreeModule"],
            src_app_dashboard_dashboard_routing_module__WEBPACK_IMPORTED_MODULE_46__["DashboardRoutingModule"]
        ]
    })
], DashboardModule);



/***/ }),

/***/ "./src/app/dashboard/element-list/element-list.component.css":
/*!*******************************************************************!*\
  !*** ./src/app/dashboard/element-list/element-list.component.css ***!
  \*******************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2Rhc2hib2FyZC9lbGVtZW50LWxpc3QvZWxlbWVudC1saXN0LmNvbXBvbmVudC5jc3MifQ== */");

/***/ }),

/***/ "./src/app/dashboard/element-list/element-list.component.ts":
/*!******************************************************************!*\
  !*** ./src/app/dashboard/element-list/element-list.component.ts ***!
  \******************************************************************/
/*! exports provided: ElementListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ElementListComponent", function() { return ElementListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ElementListComponent = class ElementListComponent {
    constructor() { }
    ngOnInit() {
    }
};
ElementListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-element-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./element-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/element-list/element-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./element-list.component.css */ "./src/app/dashboard/element-list/element-list.component.css")).default]
    })
], ElementListComponent);



/***/ }),

/***/ "./src/app/dashboard/element/element.component.css":
/*!*********************************************************!*\
  !*** ./src/app/dashboard/element/element.component.css ***!
  \*********************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2Rhc2hib2FyZC9lbGVtZW50L2VsZW1lbnQuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/dashboard/element/element.component.ts":
/*!********************************************************!*\
  !*** ./src/app/dashboard/element/element.component.ts ***!
  \********************************************************/
/*! exports provided: ElementComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ElementComponent", function() { return ElementComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ElementComponent = class ElementComponent {
    constructor() { }
    ngOnInit() {
    }
};
ElementComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-element',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./element.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/element/element.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./element.component.css */ "./src/app/dashboard/element/element.component.css")).default]
    })
], ElementComponent);



/***/ }),

/***/ "./src/app/dashboard/layout-list/layout-list.component.css":
/*!*****************************************************************!*\
  !*** ./src/app/dashboard/layout-list/layout-list.component.css ***!
  \*****************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (".items {\r\n  margin: 0 0 2em 0;\r\n  list-style-type: none;\r\n  padding: 0;\r\n  width: 24em;\r\n}\r\n\r\n  .items li {\r\n    cursor: pointer;\r\n    position: relative;\r\n    left: 0;\r\n    background-color: #EEE;\r\n    margin: .5em;\r\n    padding: .3em 0;\r\n    height: 1.6em;\r\n    border-radius: 4px;\r\n  }\r\n\r\n  .items li a {\r\n      display: block;\r\n      text-decoration: none;\r\n    }\r\n\r\n  .items li:hover {\r\n      color: #607D8B;\r\n      background-color: #DDD;\r\n      left: .1em;\r\n    }\r\n\r\n  .items li.selected {\r\n      background-color: #CFD8DC;\r\n      color: white;\r\n    }\r\n\r\n  .items li.selected:hover {\r\n        background-color: #BBD8DC;\r\n      }\r\n\r\n  .items .text {\r\n    position: relative;\r\n    top: -3px;\r\n  }\r\n\r\n  .items .badge {\r\n    display: inline-block;\r\n    font-size: small;\r\n    color: white;\r\n    padding: 0.8em 0.7em 0 0.7em;\r\n    background-color: #607D8B;\r\n    line-height: 1em;\r\n    position: relative;\r\n    left: -1px;\r\n    top: -4px;\r\n    height: 1.8em;\r\n    margin-right: .8em;\r\n    border-radius: 4px 0 0 4px;\r\n  }\r\n\r\n  .mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvZGFzaGJvYXJkL2xheW91dC1saXN0L2xheW91dC1saXN0LmNvbXBvbmVudC5jc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUE7RUFDRSxpQkFBaUI7RUFDakIscUJBQXFCO0VBQ3JCLFVBQVU7RUFDVixXQUFXO0FBQ2I7O0VBRUU7SUFDRSxlQUFlO0lBQ2Ysa0JBQWtCO0lBQ2xCLE9BQU87SUFDUCxzQkFBc0I7SUFDdEIsWUFBWTtJQUNaLGVBQWU7SUFDZixhQUFhO0lBQ2Isa0JBQWtCO0VBQ3BCOztFQUVFO01BQ0UsY0FBYztNQUNkLHFCQUFxQjtJQUN2Qjs7RUFFQTtNQUNFLGNBQWM7TUFDZCxzQkFBc0I7TUFDdEIsVUFBVTtJQUNaOztFQUVBO01BQ0UseUJBQXlCO01BQ3pCLFlBQVk7SUFDZDs7RUFFRTtRQUNFLHlCQUF5QjtNQUMzQjs7RUFFSjtJQUNFLGtCQUFrQjtJQUNsQixTQUFTO0VBQ1g7O0VBRUE7SUFDRSxxQkFBcUI7SUFDckIsZ0JBQWdCO0lBQ2hCLFlBQVk7SUFDWiw0QkFBNEI7SUFDNUIseUJBQXlCO0lBQ3pCLGdCQUFnQjtJQUNoQixrQkFBa0I7SUFDbEIsVUFBVTtJQUNWLFNBQVM7SUFDVCxhQUFhO0lBQ2Isa0JBQWtCO0lBQ2xCLDBCQUEwQjtFQUM1Qjs7RUFFRjtFQUNFLGNBQWM7RUFDZCxrQkFBa0I7QUFDcEIiLCJmaWxlIjoic3JjL2FwcC9kYXNoYm9hcmQvbGF5b3V0LWxpc3QvbGF5b3V0LWxpc3QuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIi5pdGVtcyB7XHJcbiAgbWFyZ2luOiAwIDAgMmVtIDA7XHJcbiAgbGlzdC1zdHlsZS10eXBlOiBub25lO1xyXG4gIHBhZGRpbmc6IDA7XHJcbiAgd2lkdGg6IDI0ZW07XHJcbn1cclxuXHJcbiAgLml0ZW1zIGxpIHtcclxuICAgIGN1cnNvcjogcG9pbnRlcjtcclxuICAgIHBvc2l0aW9uOiByZWxhdGl2ZTtcclxuICAgIGxlZnQ6IDA7XHJcbiAgICBiYWNrZ3JvdW5kLWNvbG9yOiAjRUVFO1xyXG4gICAgbWFyZ2luOiAuNWVtO1xyXG4gICAgcGFkZGluZzogLjNlbSAwO1xyXG4gICAgaGVpZ2h0OiAxLjZlbTtcclxuICAgIGJvcmRlci1yYWRpdXM6IDRweDtcclxuICB9XHJcblxyXG4gICAgLml0ZW1zIGxpIGEge1xyXG4gICAgICBkaXNwbGF5OiBibG9jaztcclxuICAgICAgdGV4dC1kZWNvcmF0aW9uOiBub25lO1xyXG4gICAgfVxyXG5cclxuICAgIC5pdGVtcyBsaTpob3ZlciB7XHJcbiAgICAgIGNvbG9yOiAjNjA3RDhCO1xyXG4gICAgICBiYWNrZ3JvdW5kLWNvbG9yOiAjREREO1xyXG4gICAgICBsZWZ0OiAuMWVtO1xyXG4gICAgfVxyXG5cclxuICAgIC5pdGVtcyBsaS5zZWxlY3RlZCB7XHJcbiAgICAgIGJhY2tncm91bmQtY29sb3I6ICNDRkQ4REM7XHJcbiAgICAgIGNvbG9yOiB3aGl0ZTtcclxuICAgIH1cclxuXHJcbiAgICAgIC5pdGVtcyBsaS5zZWxlY3RlZDpob3ZlciB7XHJcbiAgICAgICAgYmFja2dyb3VuZC1jb2xvcjogI0JCRDhEQztcclxuICAgICAgfVxyXG5cclxuICAuaXRlbXMgLnRleHQge1xyXG4gICAgcG9zaXRpb246IHJlbGF0aXZlO1xyXG4gICAgdG9wOiAtM3B4O1xyXG4gIH1cclxuXHJcbiAgLml0ZW1zIC5iYWRnZSB7XHJcbiAgICBkaXNwbGF5OiBpbmxpbmUtYmxvY2s7XHJcbiAgICBmb250LXNpemU6IHNtYWxsO1xyXG4gICAgY29sb3I6IHdoaXRlO1xyXG4gICAgcGFkZGluZzogMC44ZW0gMC43ZW0gMCAwLjdlbTtcclxuICAgIGJhY2tncm91bmQtY29sb3I6ICM2MDdEOEI7XHJcbiAgICBsaW5lLWhlaWdodDogMWVtO1xyXG4gICAgcG9zaXRpb246IHJlbGF0aXZlO1xyXG4gICAgbGVmdDogLTFweDtcclxuICAgIHRvcDogLTRweDtcclxuICAgIGhlaWdodDogMS44ZW07XHJcbiAgICBtYXJnaW4tcmlnaHQ6IC44ZW07XHJcbiAgICBib3JkZXItcmFkaXVzOiA0cHggMCAwIDRweDtcclxuICB9XHJcblxyXG4ubWF0LWNhcmQtc3VidGl0bGUge1xyXG4gIGRpc3BsYXk6IGJsb2NrO1xyXG4gIG1hcmdpbi1ib3R0b206IDBweDtcclxufVxyXG4iXX0= */");

/***/ }),

/***/ "./src/app/dashboard/layout-list/layout-list.component.ts":
/*!****************************************************************!*\
  !*** ./src/app/dashboard/layout-list/layout-list.component.ts ***!
  \****************************************************************/
/*! exports provided: LayoutListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "LayoutListComponent", function() { return LayoutListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let LayoutListComponent = class LayoutListComponent {
    constructor() { }
    ngOnInit() {
    }
};
LayoutListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-layout-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./layout-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/layout-list/layout-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./layout-list.component.css */ "./src/app/dashboard/layout-list/layout-list.component.css")).default]
    })
], LayoutListComponent);



/***/ }),

/***/ "./src/app/dashboard/layout/layout.component.css":
/*!*******************************************************!*\
  !*** ./src/app/dashboard/layout/layout.component.css ***!
  \*******************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2Rhc2hib2FyZC9sYXlvdXQvbGF5b3V0LmNvbXBvbmVudC5jc3MifQ== */");

/***/ }),

/***/ "./src/app/dashboard/layout/layout.component.ts":
/*!******************************************************!*\
  !*** ./src/app/dashboard/layout/layout.component.ts ***!
  \******************************************************/
/*! exports provided: LayoutComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "LayoutComponent", function() { return LayoutComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let LayoutComponent = class LayoutComponent {
    constructor() { }
    ngOnInit() {
    }
};
LayoutComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-layout',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./layout.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/layout/layout.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./layout.component.css */ "./src/app/dashboard/layout/layout.component.css")).default]
    })
], LayoutComponent);



/***/ }),

/***/ "./src/app/dashboard/tab-list/tab-list.component.css":
/*!***********************************************************!*\
  !*** ./src/app/dashboard/tab-list/tab-list.component.css ***!
  \***********************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2Rhc2hib2FyZC90YWItbGlzdC90YWItbGlzdC5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/dashboard/tab-list/tab-list.component.ts":
/*!**********************************************************!*\
  !*** ./src/app/dashboard/tab-list/tab-list.component.ts ***!
  \**********************************************************/
/*! exports provided: TabListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TabListComponent", function() { return TabListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let TabListComponent = class TabListComponent {
    constructor() { }
    ngOnInit() {
    }
};
TabListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-tab-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./tab-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/tab-list/tab-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./tab-list.component.css */ "./src/app/dashboard/tab-list/tab-list.component.css")).default]
    })
], TabListComponent);



/***/ }),

/***/ "./src/app/dashboard/tab/tab.component.css":
/*!*************************************************!*\
  !*** ./src/app/dashboard/tab/tab.component.css ***!
  \*************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2Rhc2hib2FyZC90YWIvdGFiLmNvbXBvbmVudC5jc3MifQ== */");

/***/ }),

/***/ "./src/app/dashboard/tab/tab.component.ts":
/*!************************************************!*\
  !*** ./src/app/dashboard/tab/tab.component.ts ***!
  \************************************************/
/*! exports provided: TabComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TabComponent", function() { return TabComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let TabComponent = class TabComponent {
    constructor() { }
    ngOnInit() {
    }
};
TabComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-tab',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./tab.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/dashboard/tab/tab.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./tab.component.css */ "./src/app/dashboard/tab/tab.component.css")).default]
    })
], TabComponent);



/***/ }),

/***/ "./src/app/detail/detail-routing.module.ts":
/*!*************************************************!*\
  !*** ./src/app/detail/detail-routing.module.ts ***!
  \*************************************************/
/*! exports provided: DetailRoutingModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DetailRoutingModule", function() { return DetailRoutingModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var src_app_detail_value_log_list_value_log_list_component__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! src/app/detail/value-log-list/value-log-list.component */ "./src/app/detail/value-log-list/value-log-list.component.ts");
/* harmony import */ var src_app_detail_value_history_list_value_history_list_component__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! src/app/detail/value-history-list/value-history-list.component */ "./src/app/detail/value-history-list/value-history-list.component.ts");





const detailRoutes = [
    { path: 'Values/:id/ValueLogs', component: src_app_detail_value_log_list_value_log_list_component__WEBPACK_IMPORTED_MODULE_3__["ValueLogListComponent"] },
    { path: 'Values/:id/ValueHistorys', component: src_app_detail_value_history_list_value_history_list_component__WEBPACK_IMPORTED_MODULE_4__["ValueHistoryListComponent"] }
];
let DetailRoutingModule = class DetailRoutingModule {
};
DetailRoutingModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        imports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"].forChild(detailRoutes)],
        exports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"]]
    })
], DetailRoutingModule);



/***/ }),

/***/ "./src/app/detail/detail.module.ts":
/*!*****************************************!*\
  !*** ./src/app/detail/detail.module.ts ***!
  \*****************************************/
/*! exports provided: DetailModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DetailModule", function() { return DetailModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_forms__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/forms */ "./node_modules/@angular/forms/fesm2015/forms.js");
/* harmony import */ var _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/platform-browser/animations */ "./node_modules/@angular/platform-browser/fesm2015/animations.js");
/* harmony import */ var _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! @angular/material/autocomplete */ "./node_modules/@angular/material/esm2015/autocomplete.js");
/* harmony import */ var _angular_material_badge__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! @angular/material/badge */ "./node_modules/@angular/material/esm2015/badge.js");
/* harmony import */ var _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! @angular/material/bottom-sheet */ "./node_modules/@angular/material/esm2015/bottom-sheet.js");
/* harmony import */ var _angular_material_button__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! @angular/material/button */ "./node_modules/@angular/material/esm2015/button.js");
/* harmony import */ var _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! @angular/material/button-toggle */ "./node_modules/@angular/material/esm2015/button-toggle.js");
/* harmony import */ var _angular_material_card__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! @angular/material/card */ "./node_modules/@angular/material/esm2015/card.js");
/* harmony import */ var _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_11__ = __webpack_require__(/*! @angular/material/checkbox */ "./node_modules/@angular/material/esm2015/checkbox.js");
/* harmony import */ var _angular_material_chips__WEBPACK_IMPORTED_MODULE_12__ = __webpack_require__(/*! @angular/material/chips */ "./node_modules/@angular/material/esm2015/chips.js");
/* harmony import */ var _angular_material_stepper__WEBPACK_IMPORTED_MODULE_13__ = __webpack_require__(/*! @angular/material/stepper */ "./node_modules/@angular/material/esm2015/stepper.js");
/* harmony import */ var _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_14__ = __webpack_require__(/*! @angular/material/datepicker */ "./node_modules/@angular/material/esm2015/datepicker.js");
/* harmony import */ var _angular_material_dialog__WEBPACK_IMPORTED_MODULE_15__ = __webpack_require__(/*! @angular/material/dialog */ "./node_modules/@angular/material/esm2015/dialog.js");
/* harmony import */ var _angular_material_divider__WEBPACK_IMPORTED_MODULE_16__ = __webpack_require__(/*! @angular/material/divider */ "./node_modules/@angular/material/esm2015/divider.js");
/* harmony import */ var _angular_material_expansion__WEBPACK_IMPORTED_MODULE_17__ = __webpack_require__(/*! @angular/material/expansion */ "./node_modules/@angular/material/esm2015/expansion.js");
/* harmony import */ var _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_18__ = __webpack_require__(/*! @angular/material/grid-list */ "./node_modules/@angular/material/esm2015/grid-list.js");
/* harmony import */ var _angular_material_icon__WEBPACK_IMPORTED_MODULE_19__ = __webpack_require__(/*! @angular/material/icon */ "./node_modules/@angular/material/esm2015/icon.js");
/* harmony import */ var _angular_material_input__WEBPACK_IMPORTED_MODULE_20__ = __webpack_require__(/*! @angular/material/input */ "./node_modules/@angular/material/esm2015/input.js");
/* harmony import */ var _angular_material_list__WEBPACK_IMPORTED_MODULE_21__ = __webpack_require__(/*! @angular/material/list */ "./node_modules/@angular/material/esm2015/list.js");
/* harmony import */ var _angular_material_menu__WEBPACK_IMPORTED_MODULE_22__ = __webpack_require__(/*! @angular/material/menu */ "./node_modules/@angular/material/esm2015/menu.js");
/* harmony import */ var _angular_material_core__WEBPACK_IMPORTED_MODULE_23__ = __webpack_require__(/*! @angular/material/core */ "./node_modules/@angular/material/esm2015/core.js");
/* harmony import */ var _angular_material_paginator__WEBPACK_IMPORTED_MODULE_24__ = __webpack_require__(/*! @angular/material/paginator */ "./node_modules/@angular/material/esm2015/paginator.js");
/* harmony import */ var _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_25__ = __webpack_require__(/*! @angular/material/progress-bar */ "./node_modules/@angular/material/esm2015/progress-bar.js");
/* harmony import */ var _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_26__ = __webpack_require__(/*! @angular/material/progress-spinner */ "./node_modules/@angular/material/esm2015/progress-spinner.js");
/* harmony import */ var _angular_material_radio__WEBPACK_IMPORTED_MODULE_27__ = __webpack_require__(/*! @angular/material/radio */ "./node_modules/@angular/material/esm2015/radio.js");
/* harmony import */ var _angular_material_select__WEBPACK_IMPORTED_MODULE_28__ = __webpack_require__(/*! @angular/material/select */ "./node_modules/@angular/material/esm2015/select.js");
/* harmony import */ var _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_29__ = __webpack_require__(/*! @angular/material/sidenav */ "./node_modules/@angular/material/esm2015/sidenav.js");
/* harmony import */ var _angular_material_slider__WEBPACK_IMPORTED_MODULE_30__ = __webpack_require__(/*! @angular/material/slider */ "./node_modules/@angular/material/esm2015/slider.js");
/* harmony import */ var _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_31__ = __webpack_require__(/*! @angular/material/slide-toggle */ "./node_modules/@angular/material/esm2015/slide-toggle.js");
/* harmony import */ var _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_32__ = __webpack_require__(/*! @angular/material/snack-bar */ "./node_modules/@angular/material/esm2015/snack-bar.js");
/* harmony import */ var _angular_material_sort__WEBPACK_IMPORTED_MODULE_33__ = __webpack_require__(/*! @angular/material/sort */ "./node_modules/@angular/material/esm2015/sort.js");
/* harmony import */ var _angular_material_table__WEBPACK_IMPORTED_MODULE_34__ = __webpack_require__(/*! @angular/material/table */ "./node_modules/@angular/material/esm2015/table.js");
/* harmony import */ var _angular_material_tabs__WEBPACK_IMPORTED_MODULE_35__ = __webpack_require__(/*! @angular/material/tabs */ "./node_modules/@angular/material/esm2015/tabs.js");
/* harmony import */ var _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_36__ = __webpack_require__(/*! @angular/material/toolbar */ "./node_modules/@angular/material/esm2015/toolbar.js");
/* harmony import */ var _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_37__ = __webpack_require__(/*! @angular/material/tooltip */ "./node_modules/@angular/material/esm2015/tooltip.js");
/* harmony import */ var _angular_material_tree__WEBPACK_IMPORTED_MODULE_38__ = __webpack_require__(/*! @angular/material/tree */ "./node_modules/@angular/material/esm2015/tree.js");
/* harmony import */ var src_app_detail_value_value_component__WEBPACK_IMPORTED_MODULE_39__ = __webpack_require__(/*! src/app/detail/value/value.component */ "./src/app/detail/value/value.component.ts");
/* harmony import */ var src_app_detail_value_log_value_log_component__WEBPACK_IMPORTED_MODULE_40__ = __webpack_require__(/*! src/app/detail/value-log/value-log.component */ "./src/app/detail/value-log/value-log.component.ts");
/* harmony import */ var src_app_detail_value_log_list_value_log_list_component__WEBPACK_IMPORTED_MODULE_41__ = __webpack_require__(/*! src/app/detail/value-log-list/value-log-list.component */ "./src/app/detail/value-log-list/value-log-list.component.ts");
/* harmony import */ var src_app_detail_value_history_value_history_component__WEBPACK_IMPORTED_MODULE_42__ = __webpack_require__(/*! src/app/detail/value-history/value-history.component */ "./src/app/detail/value-history/value-history.component.ts");
/* harmony import */ var src_app_detail_value_history_list_value_history_list_component__WEBPACK_IMPORTED_MODULE_43__ = __webpack_require__(/*! src/app/detail/value-history-list/value-history-list.component */ "./src/app/detail/value-history-list/value-history-list.component.ts");
/* harmony import */ var src_app_detail_detail_routing_module__WEBPACK_IMPORTED_MODULE_44__ = __webpack_require__(/*! src/app/detail/detail-routing.module */ "./src/app/detail/detail-routing.module.ts");







































// Detail






let DetailModule = class DetailModule {
};
DetailModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        declarations: [
            src_app_detail_value_value_component__WEBPACK_IMPORTED_MODULE_39__["ValueComponent"],
            src_app_detail_value_log_value_log_component__WEBPACK_IMPORTED_MODULE_40__["ValueLogComponent"],
            src_app_detail_value_log_list_value_log_list_component__WEBPACK_IMPORTED_MODULE_41__["ValueLogListComponent"],
            src_app_detail_value_history_value_history_component__WEBPACK_IMPORTED_MODULE_42__["ValueHistoryComponent"],
            src_app_detail_value_history_list_value_history_list_component__WEBPACK_IMPORTED_MODULE_43__["ValueHistoryListComponent"],
        ],
        imports: [
            _angular_common__WEBPACK_IMPORTED_MODULE_2__["CommonModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["FormsModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["ReactiveFormsModule"],
            _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_4__["BrowserAnimationsModule"],
            _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_5__["MatAutocompleteModule"],
            _angular_material_badge__WEBPACK_IMPORTED_MODULE_6__["MatBadgeModule"],
            _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_7__["MatBottomSheetModule"],
            _angular_material_button__WEBPACK_IMPORTED_MODULE_8__["MatButtonModule"],
            _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_9__["MatButtonToggleModule"],
            _angular_material_card__WEBPACK_IMPORTED_MODULE_10__["MatCardModule"],
            _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_11__["MatCheckboxModule"],
            _angular_material_chips__WEBPACK_IMPORTED_MODULE_12__["MatChipsModule"],
            _angular_material_stepper__WEBPACK_IMPORTED_MODULE_13__["MatStepperModule"],
            _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_14__["MatDatepickerModule"],
            _angular_material_dialog__WEBPACK_IMPORTED_MODULE_15__["MatDialogModule"],
            _angular_material_divider__WEBPACK_IMPORTED_MODULE_16__["MatDividerModule"],
            _angular_material_expansion__WEBPACK_IMPORTED_MODULE_17__["MatExpansionModule"],
            _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_18__["MatGridListModule"],
            _angular_material_icon__WEBPACK_IMPORTED_MODULE_19__["MatIconModule"],
            _angular_material_input__WEBPACK_IMPORTED_MODULE_20__["MatInputModule"],
            _angular_material_list__WEBPACK_IMPORTED_MODULE_21__["MatListModule"],
            _angular_material_menu__WEBPACK_IMPORTED_MODULE_22__["MatMenuModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_23__["MatNativeDateModule"],
            _angular_material_paginator__WEBPACK_IMPORTED_MODULE_24__["MatPaginatorModule"],
            _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_25__["MatProgressBarModule"],
            _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_26__["MatProgressSpinnerModule"],
            _angular_material_radio__WEBPACK_IMPORTED_MODULE_27__["MatRadioModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_23__["MatRippleModule"],
            _angular_material_select__WEBPACK_IMPORTED_MODULE_28__["MatSelectModule"],
            _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_29__["MatSidenavModule"],
            _angular_material_slider__WEBPACK_IMPORTED_MODULE_30__["MatSliderModule"],
            _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_31__["MatSlideToggleModule"],
            _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_32__["MatSnackBarModule"],
            _angular_material_sort__WEBPACK_IMPORTED_MODULE_33__["MatSortModule"],
            _angular_material_table__WEBPACK_IMPORTED_MODULE_34__["MatTableModule"],
            _angular_material_tabs__WEBPACK_IMPORTED_MODULE_35__["MatTabsModule"],
            _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_36__["MatToolbarModule"],
            _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_37__["MatTooltipModule"],
            _angular_material_tree__WEBPACK_IMPORTED_MODULE_38__["MatTreeModule"],
            src_app_detail_detail_routing_module__WEBPACK_IMPORTED_MODULE_44__["DetailRoutingModule"]
        ]
    })
], DetailModule);



/***/ }),

/***/ "./src/app/detail/value-history-list/value-history-list.component.css":
/*!****************************************************************************!*\
  !*** ./src/app/detail/value-history-list/value-history-list.component.css ***!
  \****************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 140px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvZGV0YWlsL3ZhbHVlLWhpc3RvcnktbGlzdC92YWx1ZS1oaXN0b3J5LWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9kZXRhaWwvdmFsdWUtaGlzdG9yeS1saXN0L3ZhbHVlLWhpc3RvcnktbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTQwcHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/detail/value-history-list/value-history-list.component.ts":
/*!***************************************************************************!*\
  !*** ./src/app/detail/value-history-list/value-history-list.component.ts ***!
  \***************************************************************************/
/*! exports provided: ValueHistoryListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueHistoryListComponent", function() { return ValueHistoryListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _value_history_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ../value-history.service */ "./src/app/detail/value-history.service.ts");






let ValueHistoryListComponent = class ValueHistoryListComponent {
    constructor(valueService, snackBar, location, route) {
        this.valueService = valueService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.ValueHistorys = [];
        this.tableColumns = ['Timestamp', 'Message'];
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.valueService.getValueHistorys(this.route.snapshot.params['id']).subscribe((ValueHistorysMsg) => {
            this.ValueHistorys = ValueHistorysMsg.ValueHistorys;
            this.snackBar.open(ValueHistorysMsg.Count + ' Value History entries loaded. ', '', { duration: 3000 });
            console.log(this.ValueHistorys);
            return this.ValueHistorys;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Value History entries found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
ValueHistoryListComponent.ctorParameters = () => [
    { type: _value_history_service__WEBPACK_IMPORTED_MODULE_5__["ValueHistoryService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
ValueHistoryListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-history-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-history-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-history-list/value-history-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-history-list.component.css */ "./src/app/detail/value-history-list/value-history-list.component.css")).default]
    })
], ValueHistoryListComponent);



/***/ }),

/***/ "./src/app/detail/value-history.service.ts":
/*!*************************************************!*\
  !*** ./src/app/detail/value-history.service.ts ***!
  \*************************************************/
/*! exports provided: ValueHistoryService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueHistoryService", function() { return ValueHistoryService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let ValueHistoryService = class ValueHistoryService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getValueHistorys(ValueID) {
        if (typeof (ValueID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'ValueHistorys');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Values/' + ValueID + '/ValueHistorys');
        }
    }
    getValueHistory(ValueId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'ValueHistorys'}/${ValueId}`);
    }
};
ValueHistoryService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
ValueHistoryService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], ValueHistoryService);



/***/ }),

/***/ "./src/app/detail/value-history/value-history.component.css":
/*!******************************************************************!*\
  !*** ./src/app/detail/value-history/value-history.component.css ***!
  \******************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2RldGFpbC92YWx1ZS1oaXN0b3J5L3ZhbHVlLWhpc3RvcnkuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/detail/value-history/value-history.component.ts":
/*!*****************************************************************!*\
  !*** ./src/app/detail/value-history/value-history.component.ts ***!
  \*****************************************************************/
/*! exports provided: ValueHistoryComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueHistoryComponent", function() { return ValueHistoryComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ValueHistoryComponent = class ValueHistoryComponent {
    constructor() { }
    ngOnInit() {
    }
};
ValueHistoryComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-history',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-history.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-history/value-history.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-history.component.css */ "./src/app/detail/value-history/value-history.component.css")).default]
    })
], ValueHistoryComponent);



/***/ }),

/***/ "./src/app/detail/value-log-list/value-log-list.component.css":
/*!********************************************************************!*\
  !*** ./src/app/detail/value-log-list/value-log-list.component.css ***!
  \********************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 140px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvZGV0YWlsL3ZhbHVlLWxvZy1saXN0L3ZhbHVlLWxvZy1saXN0LmNvbXBvbmVudC5jc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUE7RUFDRSxXQUFXO0VBQ1gsMkJBQTJCO0FBQzdCOztBQUVBO0VBQ0UsMkJBQTJCO0VBQzNCLGtCQUFrQjtBQUNwQjs7QUFFQTtFQUNFLGNBQWM7RUFDZCxrQkFBa0I7RUFDbEIsZ0JBQWdCO0FBQ2xCIiwiZmlsZSI6InNyYy9hcHAvZGV0YWlsL3ZhbHVlLWxvZy1saXN0L3ZhbHVlLWxvZy1saXN0LmNvbXBvbmVudC5jc3MiLCJzb3VyY2VzQ29udGVudCI6WyI6aG9zdCB7XHJcbiAgd2lkdGg6IDEwMCU7XHJcbiAgbWFyZ2luOiAxMnB4IGF1dG8gYXV0byBhdXRvO1xyXG59XHJcblxyXG4ubWF0LXJvdzpob3ZlciB7XHJcbiAgYmFja2dyb3VuZC1jb2xvcjogbGlnaHRncmF5O1xyXG4gIGJvcmRlci1yYWRpdXM6IDRweDtcclxufVxyXG5cclxuLm1hdC1jYXJkLXN1YnRpdGxlIHtcclxuICBkaXNwbGF5OiBibG9jaztcclxuICBtYXJnaW4tYm90dG9tOiAwcHg7XHJcbiAgbWluLXdpZHRoOiAxNDBweDtcclxufVxyXG4iXX0= */");

/***/ }),

/***/ "./src/app/detail/value-log-list/value-log-list.component.ts":
/*!*******************************************************************!*\
  !*** ./src/app/detail/value-log-list/value-log-list.component.ts ***!
  \*******************************************************************/
/*! exports provided: ValueLogListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueLogListComponent", function() { return ValueLogListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _value_log_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ../value-log.service */ "./src/app/detail/value-log.service.ts");






let ValueLogListComponent = class ValueLogListComponent {
    constructor(valueService, snackBar, location, route) {
        this.valueService = valueService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.ValueLogs = [];
        this.tableColumns = ['Timestamp', 'Message'];
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.valueService.getValueLogs(this.route.snapshot.params['id']).subscribe((ValueLogsMsg) => {
            this.ValueLogs = ValueLogsMsg.ValueLogs;
            this.snackBar.open(ValueLogsMsg.Count + ' Value Log entries loaded. ', '', { duration: 3000 });
            console.log(this.ValueLogs);
            return this.ValueLogs;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Value Log entries found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
ValueLogListComponent.ctorParameters = () => [
    { type: _value_log_service__WEBPACK_IMPORTED_MODULE_5__["ValueLogService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
ValueLogListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-log-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-log-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-log-list/value-log-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-log-list.component.css */ "./src/app/detail/value-log-list/value-log-list.component.css")).default]
    })
], ValueLogListComponent);



/***/ }),

/***/ "./src/app/detail/value-log.service.ts":
/*!*********************************************!*\
  !*** ./src/app/detail/value-log.service.ts ***!
  \*********************************************/
/*! exports provided: ValueLogService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueLogService", function() { return ValueLogService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let ValueLogService = class ValueLogService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getValueLogs(ValueID) {
        if (typeof (ValueID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'ValueLogs');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Values/' + ValueID + '/ValueLogs');
        }
    }
    getValueLog(ValueId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'ValueLogs'}/${ValueId}`);
    }
};
ValueLogService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
ValueLogService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], ValueLogService);



/***/ }),

/***/ "./src/app/detail/value-log/value-log.component.css":
/*!**********************************************************!*\
  !*** ./src/app/detail/value-log/value-log.component.css ***!
  \**********************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2RldGFpbC92YWx1ZS1sb2cvdmFsdWUtbG9nLmNvbXBvbmVudC5jc3MifQ== */");

/***/ }),

/***/ "./src/app/detail/value-log/value-log.component.ts":
/*!*********************************************************!*\
  !*** ./src/app/detail/value-log/value-log.component.ts ***!
  \*********************************************************/
/*! exports provided: ValueLogComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueLogComponent", function() { return ValueLogComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ValueLogComponent = class ValueLogComponent {
    constructor() { }
    ngOnInit() {
    }
};
ValueLogComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-log',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-log.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value-log/value-log.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-log.component.css */ "./src/app/detail/value-log/value-log.component.css")).default]
    })
], ValueLogComponent);



/***/ }),

/***/ "./src/app/detail/value.service.ts":
/*!*****************************************!*\
  !*** ./src/app/detail/value.service.ts ***!
  \*****************************************/
/*! exports provided: ValueService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueService", function() { return ValueService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let ValueService = class ValueService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getValues(DeviceID) {
        if (typeof (DeviceID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Values');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Devices/' + DeviceID + '/Values');
        }
    }
    getValue(ValueID) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Values'}/${ValueID}`);
    }
};
ValueService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
ValueService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], ValueService);



/***/ }),

/***/ "./src/app/detail/value/value.component.css":
/*!**************************************************!*\
  !*** ./src/app/detail/value/value.component.css ***!
  \**************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2RldGFpbC92YWx1ZS92YWx1ZS5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/detail/value/value.component.ts":
/*!*************************************************!*\
  !*** ./src/app/detail/value/value.component.ts ***!
  \*************************************************/
/*! exports provided: ValueComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueComponent", function() { return ValueComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ValueComponent = class ValueComponent {
    constructor() { }
    ngOnInit() {
    }
};
ValueComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/detail/value/value.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value.component.css */ "./src/app/detail/value/value.component.css")).default]
    })
], ValueComponent);



/***/ }),

/***/ "./src/app/hardware/device-list/device-list.component.css":
/*!****************************************************************!*\
  !*** ./src/app/hardware/device-list/device-list.component.css ***!
  \****************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 130px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaGFyZHdhcmUvZGV2aWNlLWxpc3QvZGV2aWNlLWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9oYXJkd2FyZS9kZXZpY2UtbGlzdC9kZXZpY2UtbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTMwcHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/hardware/device-list/device-list.component.ts":
/*!***************************************************************!*\
  !*** ./src/app/hardware/device-list/device-list.component.ts ***!
  \***************************************************************/
/*! exports provided: DeviceListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DeviceListComponent", function() { return DeviceListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _device_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ../device.service */ "./src/app/hardware/device.service.ts");






let DeviceListComponent = class DeviceListComponent {
    constructor(deviceService, snackBar, location, route) {
        this.deviceService = deviceService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.Devices = [];
        this.tableColumns = ['DeviceID', 'InterfaceID', 'Name', 'ExternalID', 'Timestamp'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.deviceService.getDevices(this.route.snapshot.params['id']).subscribe((DevicesMsg) => {
            this.Devices = DevicesMsg.Devices;
            this.snackBar.open(DevicesMsg.Count + ' Devices loaded. ', '', { duration: 3000 });
            console.log(this.Devices);
            return this.Devices;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Devices found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
DeviceListComponent.ctorParameters = () => [
    { type: _device_service__WEBPACK_IMPORTED_MODULE_5__["DeviceService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
DeviceListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-device-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./device-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/device-list/device-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./device-list.component.css */ "./src/app/hardware/device-list/device-list.component.css")).default]
    })
], DeviceListComponent);



/***/ }),

/***/ "./src/app/hardware/device.service.ts":
/*!********************************************!*\
  !*** ./src/app/hardware/device.service.ts ***!
  \********************************************/
/*! exports provided: DeviceService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DeviceService", function() { return DeviceService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let DeviceService = class DeviceService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getDevices(InterfaceID) {
        if (typeof (InterfaceID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Devices');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Interfaces/' + InterfaceID + '/Devices');
        }
    }
    getDevice(DeviceID) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Devices'}/${DeviceID}`);
    }
};
DeviceService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
DeviceService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], DeviceService);



/***/ }),

/***/ "./src/app/hardware/device/device.component.css":
/*!******************************************************!*\
  !*** ./src/app/hardware/device/device.component.css ***!
  \******************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2hhcmR3YXJlL2RldmljZS9kZXZpY2UuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/hardware/device/device.component.ts":
/*!*****************************************************!*\
  !*** ./src/app/hardware/device/device.component.ts ***!
  \*****************************************************/
/*! exports provided: DeviceComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "DeviceComponent", function() { return DeviceComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let DeviceComponent = class DeviceComponent {
    constructor() { }
    ngOnInit() {
    }
};
DeviceComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-device',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./device.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/device/device.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./device.component.css */ "./src/app/hardware/device/device.component.css")).default]
    })
], DeviceComponent);



/***/ }),

/***/ "./src/app/hardware/hardware-routing.module.ts":
/*!*****************************************************!*\
  !*** ./src/app/hardware/hardware-routing.module.ts ***!
  \*****************************************************/
/*! exports provided: HardwareRoutingModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "HardwareRoutingModule", function() { return HardwareRoutingModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var src_app_hardware_interface_interface_component__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! src/app/hardware/interface/interface.component */ "./src/app/hardware/interface/interface.component.ts");
/* harmony import */ var src_app_hardware_interface_list_interface_list_component__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! src/app/hardware/interface-list/interface-list.component */ "./src/app/hardware/interface-list/interface-list.component.ts");
/* harmony import */ var src_app_hardware_interface_log_list_interface_log_list_component__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! src/app/hardware/interface-log-list/interface-log-list.component */ "./src/app/hardware/interface-log-list/interface-log-list.component.ts");
/* harmony import */ var src_app_hardware_device_list_device_list_component__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! src/app/hardware/device-list/device-list.component */ "./src/app/hardware/device-list/device-list.component.ts");
/* harmony import */ var src_app_hardware_value_list_value_list_component__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! src/app/hardware/value-list/value-list.component */ "./src/app/hardware/value-list/value-list.component.ts");
/* harmony import */ var src_app_hardware_value_script_list_value_script_list_component__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! src/app/hardware/value-script-list/value-script-list.component */ "./src/app/hardware/value-script-list/value-script-list.component.ts");









const interfaceRoutes = [
    { path: 'Interfaces', component: src_app_hardware_interface_list_interface_list_component__WEBPACK_IMPORTED_MODULE_4__["InterfaceListComponent"] },
    { path: 'Interfaces/:id', component: src_app_hardware_interface_interface_component__WEBPACK_IMPORTED_MODULE_3__["InterfaceComponent"] },
    { path: 'Interfaces/:id/InterfaceLogs', component: src_app_hardware_interface_log_list_interface_log_list_component__WEBPACK_IMPORTED_MODULE_5__["InterfaceLogListComponent"] },
    { path: 'Interfaces/:id/Devices', component: src_app_hardware_device_list_device_list_component__WEBPACK_IMPORTED_MODULE_6__["DeviceListComponent"] },
    { path: 'Devices/:id/Values', component: src_app_hardware_value_list_value_list_component__WEBPACK_IMPORTED_MODULE_7__["ValueListComponent"] },
    { path: 'Values/:id/ValueScripts', component: src_app_hardware_value_script_list_value_script_list_component__WEBPACK_IMPORTED_MODULE_8__["ValueScriptListComponent"] }
];
let HardwareRoutingModule = class HardwareRoutingModule {
};
HardwareRoutingModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        imports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"].forChild(interfaceRoutes)],
        exports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"]]
    })
], HardwareRoutingModule);



/***/ }),

/***/ "./src/app/hardware/hardware.module.ts":
/*!*********************************************!*\
  !*** ./src/app/hardware/hardware.module.ts ***!
  \*********************************************/
/*! exports provided: HardwareModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "HardwareModule", function() { return HardwareModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_forms__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/forms */ "./node_modules/@angular/forms/fesm2015/forms.js");
/* harmony import */ var _angular_platform_browser__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/platform-browser */ "./node_modules/@angular/platform-browser/fesm2015/platform-browser.js");
/* harmony import */ var _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! @angular/platform-browser/animations */ "./node_modules/@angular/platform-browser/fesm2015/animations.js");
/* harmony import */ var _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! @angular/material/autocomplete */ "./node_modules/@angular/material/esm2015/autocomplete.js");
/* harmony import */ var _angular_material_badge__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! @angular/material/badge */ "./node_modules/@angular/material/esm2015/badge.js");
/* harmony import */ var _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! @angular/material/bottom-sheet */ "./node_modules/@angular/material/esm2015/bottom-sheet.js");
/* harmony import */ var _angular_material_button__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! @angular/material/button */ "./node_modules/@angular/material/esm2015/button.js");
/* harmony import */ var _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! @angular/material/button-toggle */ "./node_modules/@angular/material/esm2015/button-toggle.js");
/* harmony import */ var _angular_material_card__WEBPACK_IMPORTED_MODULE_11__ = __webpack_require__(/*! @angular/material/card */ "./node_modules/@angular/material/esm2015/card.js");
/* harmony import */ var _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_12__ = __webpack_require__(/*! @angular/material/checkbox */ "./node_modules/@angular/material/esm2015/checkbox.js");
/* harmony import */ var _angular_material_chips__WEBPACK_IMPORTED_MODULE_13__ = __webpack_require__(/*! @angular/material/chips */ "./node_modules/@angular/material/esm2015/chips.js");
/* harmony import */ var _angular_material_stepper__WEBPACK_IMPORTED_MODULE_14__ = __webpack_require__(/*! @angular/material/stepper */ "./node_modules/@angular/material/esm2015/stepper.js");
/* harmony import */ var _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_15__ = __webpack_require__(/*! @angular/material/datepicker */ "./node_modules/@angular/material/esm2015/datepicker.js");
/* harmony import */ var _angular_material_dialog__WEBPACK_IMPORTED_MODULE_16__ = __webpack_require__(/*! @angular/material/dialog */ "./node_modules/@angular/material/esm2015/dialog.js");
/* harmony import */ var _angular_material_divider__WEBPACK_IMPORTED_MODULE_17__ = __webpack_require__(/*! @angular/material/divider */ "./node_modules/@angular/material/esm2015/divider.js");
/* harmony import */ var _angular_material_expansion__WEBPACK_IMPORTED_MODULE_18__ = __webpack_require__(/*! @angular/material/expansion */ "./node_modules/@angular/material/esm2015/expansion.js");
/* harmony import */ var _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_19__ = __webpack_require__(/*! @angular/material/grid-list */ "./node_modules/@angular/material/esm2015/grid-list.js");
/* harmony import */ var _angular_material_icon__WEBPACK_IMPORTED_MODULE_20__ = __webpack_require__(/*! @angular/material/icon */ "./node_modules/@angular/material/esm2015/icon.js");
/* harmony import */ var _angular_material_input__WEBPACK_IMPORTED_MODULE_21__ = __webpack_require__(/*! @angular/material/input */ "./node_modules/@angular/material/esm2015/input.js");
/* harmony import */ var _angular_material_list__WEBPACK_IMPORTED_MODULE_22__ = __webpack_require__(/*! @angular/material/list */ "./node_modules/@angular/material/esm2015/list.js");
/* harmony import */ var _angular_material_menu__WEBPACK_IMPORTED_MODULE_23__ = __webpack_require__(/*! @angular/material/menu */ "./node_modules/@angular/material/esm2015/menu.js");
/* harmony import */ var _angular_material_core__WEBPACK_IMPORTED_MODULE_24__ = __webpack_require__(/*! @angular/material/core */ "./node_modules/@angular/material/esm2015/core.js");
/* harmony import */ var _angular_material_paginator__WEBPACK_IMPORTED_MODULE_25__ = __webpack_require__(/*! @angular/material/paginator */ "./node_modules/@angular/material/esm2015/paginator.js");
/* harmony import */ var _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_26__ = __webpack_require__(/*! @angular/material/progress-bar */ "./node_modules/@angular/material/esm2015/progress-bar.js");
/* harmony import */ var _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_27__ = __webpack_require__(/*! @angular/material/progress-spinner */ "./node_modules/@angular/material/esm2015/progress-spinner.js");
/* harmony import */ var _angular_material_radio__WEBPACK_IMPORTED_MODULE_28__ = __webpack_require__(/*! @angular/material/radio */ "./node_modules/@angular/material/esm2015/radio.js");
/* harmony import */ var _angular_material_select__WEBPACK_IMPORTED_MODULE_29__ = __webpack_require__(/*! @angular/material/select */ "./node_modules/@angular/material/esm2015/select.js");
/* harmony import */ var _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_30__ = __webpack_require__(/*! @angular/material/sidenav */ "./node_modules/@angular/material/esm2015/sidenav.js");
/* harmony import */ var _angular_material_slider__WEBPACK_IMPORTED_MODULE_31__ = __webpack_require__(/*! @angular/material/slider */ "./node_modules/@angular/material/esm2015/slider.js");
/* harmony import */ var _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_32__ = __webpack_require__(/*! @angular/material/slide-toggle */ "./node_modules/@angular/material/esm2015/slide-toggle.js");
/* harmony import */ var _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_33__ = __webpack_require__(/*! @angular/material/snack-bar */ "./node_modules/@angular/material/esm2015/snack-bar.js");
/* harmony import */ var _angular_material_sort__WEBPACK_IMPORTED_MODULE_34__ = __webpack_require__(/*! @angular/material/sort */ "./node_modules/@angular/material/esm2015/sort.js");
/* harmony import */ var _angular_material_table__WEBPACK_IMPORTED_MODULE_35__ = __webpack_require__(/*! @angular/material/table */ "./node_modules/@angular/material/esm2015/table.js");
/* harmony import */ var _angular_material_tabs__WEBPACK_IMPORTED_MODULE_36__ = __webpack_require__(/*! @angular/material/tabs */ "./node_modules/@angular/material/esm2015/tabs.js");
/* harmony import */ var _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_37__ = __webpack_require__(/*! @angular/material/toolbar */ "./node_modules/@angular/material/esm2015/toolbar.js");
/* harmony import */ var _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_38__ = __webpack_require__(/*! @angular/material/tooltip */ "./node_modules/@angular/material/esm2015/tooltip.js");
/* harmony import */ var _angular_material_tree__WEBPACK_IMPORTED_MODULE_39__ = __webpack_require__(/*! @angular/material/tree */ "./node_modules/@angular/material/esm2015/tree.js");
/* harmony import */ var src_app_hardware_interface_interface_component__WEBPACK_IMPORTED_MODULE_40__ = __webpack_require__(/*! src/app/hardware/interface/interface.component */ "./src/app/hardware/interface/interface.component.ts");
/* harmony import */ var src_app_hardware_interface_list_interface_list_component__WEBPACK_IMPORTED_MODULE_41__ = __webpack_require__(/*! src/app/hardware/interface-list/interface-list.component */ "./src/app/hardware/interface-list/interface-list.component.ts");
/* harmony import */ var src_app_hardware_interface_log_interface_log_component__WEBPACK_IMPORTED_MODULE_42__ = __webpack_require__(/*! src/app/hardware/interface-log/interface-log.component */ "./src/app/hardware/interface-log/interface-log.component.ts");
/* harmony import */ var src_app_hardware_interface_log_list_interface_log_list_component__WEBPACK_IMPORTED_MODULE_43__ = __webpack_require__(/*! src/app/hardware/interface-log-list/interface-log-list.component */ "./src/app/hardware/interface-log-list/interface-log-list.component.ts");
/* harmony import */ var src_app_hardware_device_device_component__WEBPACK_IMPORTED_MODULE_44__ = __webpack_require__(/*! src/app/hardware/device/device.component */ "./src/app/hardware/device/device.component.ts");
/* harmony import */ var src_app_hardware_device_list_device_list_component__WEBPACK_IMPORTED_MODULE_45__ = __webpack_require__(/*! src/app/hardware/device-list/device-list.component */ "./src/app/hardware/device-list/device-list.component.ts");
/* harmony import */ var src_app_hardware_value_list_value_list_component__WEBPACK_IMPORTED_MODULE_46__ = __webpack_require__(/*! src/app/hardware/value-list/value-list.component */ "./src/app/hardware/value-list/value-list.component.ts");
/* harmony import */ var src_app_hardware_value_script_value_script_component__WEBPACK_IMPORTED_MODULE_47__ = __webpack_require__(/*! src/app/hardware/value-script/value-script.component */ "./src/app/hardware/value-script/value-script.component.ts");
/* harmony import */ var src_app_hardware_value_script_list_value_script_list_component__WEBPACK_IMPORTED_MODULE_48__ = __webpack_require__(/*! src/app/hardware/value-script-list/value-script-list.component */ "./src/app/hardware/value-script-list/value-script-list.component.ts");
/* harmony import */ var src_app_hardware_hardware_routing_module__WEBPACK_IMPORTED_MODULE_49__ = __webpack_require__(/*! src/app/hardware/hardware-routing.module */ "./src/app/hardware/hardware-routing.module.ts");








































// Hardware










let HardwareModule = class HardwareModule {
};
HardwareModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        declarations: [
            src_app_hardware_interface_interface_component__WEBPACK_IMPORTED_MODULE_40__["InterfaceComponent"],
            src_app_hardware_interface_list_interface_list_component__WEBPACK_IMPORTED_MODULE_41__["InterfaceListComponent"],
            src_app_hardware_device_device_component__WEBPACK_IMPORTED_MODULE_44__["DeviceComponent"],
            src_app_hardware_device_list_device_list_component__WEBPACK_IMPORTED_MODULE_45__["DeviceListComponent"],
            src_app_hardware_interface_log_interface_log_component__WEBPACK_IMPORTED_MODULE_42__["InterfaceLogComponent"],
            src_app_hardware_interface_log_list_interface_log_list_component__WEBPACK_IMPORTED_MODULE_43__["InterfaceLogListComponent"],
            src_app_hardware_value_list_value_list_component__WEBPACK_IMPORTED_MODULE_46__["ValueListComponent"],
            src_app_hardware_value_script_value_script_component__WEBPACK_IMPORTED_MODULE_47__["ValueScriptComponent"],
            src_app_hardware_value_script_list_value_script_list_component__WEBPACK_IMPORTED_MODULE_48__["ValueScriptListComponent"]
        ],
        imports: [
            _angular_common__WEBPACK_IMPORTED_MODULE_2__["CommonModule"],
            _angular_platform_browser__WEBPACK_IMPORTED_MODULE_4__["BrowserModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["FormsModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["ReactiveFormsModule"],
            _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_5__["BrowserAnimationsModule"],
            _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_6__["MatAutocompleteModule"],
            _angular_material_badge__WEBPACK_IMPORTED_MODULE_7__["MatBadgeModule"],
            _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_8__["MatBottomSheetModule"],
            _angular_material_button__WEBPACK_IMPORTED_MODULE_9__["MatButtonModule"],
            _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_10__["MatButtonToggleModule"],
            _angular_material_card__WEBPACK_IMPORTED_MODULE_11__["MatCardModule"],
            _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_12__["MatCheckboxModule"],
            _angular_material_chips__WEBPACK_IMPORTED_MODULE_13__["MatChipsModule"],
            _angular_material_stepper__WEBPACK_IMPORTED_MODULE_14__["MatStepperModule"],
            _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_15__["MatDatepickerModule"],
            _angular_material_dialog__WEBPACK_IMPORTED_MODULE_16__["MatDialogModule"],
            _angular_material_divider__WEBPACK_IMPORTED_MODULE_17__["MatDividerModule"],
            _angular_material_expansion__WEBPACK_IMPORTED_MODULE_18__["MatExpansionModule"],
            _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_19__["MatGridListModule"],
            _angular_material_icon__WEBPACK_IMPORTED_MODULE_20__["MatIconModule"],
            _angular_material_input__WEBPACK_IMPORTED_MODULE_21__["MatInputModule"],
            _angular_material_list__WEBPACK_IMPORTED_MODULE_22__["MatListModule"],
            _angular_material_menu__WEBPACK_IMPORTED_MODULE_23__["MatMenuModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_24__["MatNativeDateModule"],
            _angular_material_paginator__WEBPACK_IMPORTED_MODULE_25__["MatPaginatorModule"],
            _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_26__["MatProgressBarModule"],
            _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_27__["MatProgressSpinnerModule"],
            _angular_material_radio__WEBPACK_IMPORTED_MODULE_28__["MatRadioModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_24__["MatRippleModule"],
            _angular_material_select__WEBPACK_IMPORTED_MODULE_29__["MatSelectModule"],
            _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_30__["MatSidenavModule"],
            _angular_material_slider__WEBPACK_IMPORTED_MODULE_31__["MatSliderModule"],
            _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_32__["MatSlideToggleModule"],
            _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_33__["MatSnackBarModule"],
            _angular_material_sort__WEBPACK_IMPORTED_MODULE_34__["MatSortModule"],
            _angular_material_table__WEBPACK_IMPORTED_MODULE_35__["MatTableModule"],
            _angular_material_tabs__WEBPACK_IMPORTED_MODULE_36__["MatTabsModule"],
            _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_37__["MatToolbarModule"],
            _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_38__["MatTooltipModule"],
            _angular_material_tree__WEBPACK_IMPORTED_MODULE_39__["MatTreeModule"],
            src_app_hardware_hardware_routing_module__WEBPACK_IMPORTED_MODULE_49__["HardwareRoutingModule"]
        ]
    })
], HardwareModule);



/***/ }),

/***/ "./src/app/hardware/interface-list/interface-list.component.css":
/*!**********************************************************************!*\
  !*** ./src/app/hardware/interface-list/interface-list.component.css ***!
  \**********************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 145px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaGFyZHdhcmUvaW50ZXJmYWNlLWxpc3QvaW50ZXJmYWNlLWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9oYXJkd2FyZS9pbnRlcmZhY2UtbGlzdC9pbnRlcmZhY2UtbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTQ1cHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/hardware/interface-list/interface-list.component.ts":
/*!*********************************************************************!*\
  !*** ./src/app/hardware/interface-list/interface-list.component.ts ***!
  \*********************************************************************/
/*! exports provided: InterfaceListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "InterfaceListComponent", function() { return InterfaceListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _interface_service__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../interface.service */ "./src/app/hardware/interface.service.ts");





let InterfaceListComponent = class InterfaceListComponent {
    constructor(interfaceService, snackBar, location) {
        this.interfaceService = interfaceService;
        this.snackBar = snackBar;
        this.location = location;
        this.Interfaces = [];
        this.tableColumns = ['InterfaceID', 'Name', 'Parameters', 'Configuration', 'Notifiable', 'Active'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.interfaceService.getInterfaces().subscribe((InterfacesMsg) => {
            this.Interfaces = InterfacesMsg.Interfaces;
            this.snackBar.open(InterfacesMsg.Count + ' Interface' + (this.Interfaces.length > 1 ? 's' : '') + ' loaded. ', '', { duration: 3000 });
            console.log(this.Interfaces);
            return this.Interfaces;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Interfaces found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
InterfaceListComponent.ctorParameters = () => [
    { type: _interface_service__WEBPACK_IMPORTED_MODULE_4__["InterfaceService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_3__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] }
];
InterfaceListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-interface-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./interface-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-list/interface-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./interface-list.component.css */ "./src/app/hardware/interface-list/interface-list.component.css")).default]
    })
], InterfaceListComponent);



/***/ }),

/***/ "./src/app/hardware/interface-log-list/interface-log-list.component.css":
/*!******************************************************************************!*\
  !*** ./src/app/hardware/interface-log-list/interface-log-list.component.css ***!
  \******************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 140px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaGFyZHdhcmUvaW50ZXJmYWNlLWxvZy1saXN0L2ludGVyZmFjZS1sb2ctbGlzdC5jb21wb25lbnQuY3NzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBO0VBQ0UsV0FBVztFQUNYLDJCQUEyQjtBQUM3Qjs7QUFFQTtFQUNFLDJCQUEyQjtFQUMzQixrQkFBa0I7QUFDcEI7O0FBRUE7RUFDRSxjQUFjO0VBQ2Qsa0JBQWtCO0VBQ2xCLGdCQUFnQjtBQUNsQiIsImZpbGUiOiJzcmMvYXBwL2hhcmR3YXJlL2ludGVyZmFjZS1sb2ctbGlzdC9pbnRlcmZhY2UtbG9nLWxpc3QuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIjpob3N0IHtcclxuICB3aWR0aDogMTAwJTtcclxuICBtYXJnaW46IDEycHggYXV0byBhdXRvIGF1dG87XHJcbn1cclxuXHJcbi5tYXQtcm93OmhvdmVyIHtcclxuICBiYWNrZ3JvdW5kLWNvbG9yOiBsaWdodGdyYXk7XHJcbiAgYm9yZGVyLXJhZGl1czogNHB4O1xyXG59XHJcblxyXG4ubWF0LWNhcmQtc3VidGl0bGUge1xyXG4gIGRpc3BsYXk6IGJsb2NrO1xyXG4gIG1hcmdpbi1ib3R0b206IDBweDtcclxuICBtaW4td2lkdGg6IDE0MHB4O1xyXG59XHJcbiJdfQ== */");

/***/ }),

/***/ "./src/app/hardware/interface-log-list/interface-log-list.component.ts":
/*!*****************************************************************************!*\
  !*** ./src/app/hardware/interface-log-list/interface-log-list.component.ts ***!
  \*****************************************************************************/
/*! exports provided: InterfaceLogListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "InterfaceLogListComponent", function() { return InterfaceLogListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _interface_log_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ../interface-log.service */ "./src/app/hardware/interface-log.service.ts");






let InterfaceLogListComponent = class InterfaceLogListComponent {
    constructor(interfaceService, snackBar, location, route) {
        this.interfaceService = interfaceService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.InterfaceLogs = [];
        this.tableColumns = ['Timestamp', 'Message'];
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.interfaceService.getInterfaceLogs(this.route.snapshot.params['id']).subscribe((InterfaceLogsMsg) => {
            this.InterfaceLogs = InterfaceLogsMsg.InterfaceLogs;
            this.snackBar.open(InterfaceLogsMsg.Count + ' Interface Log entries loaded. ', '', { duration: 3000 });
            console.log(this.InterfaceLogs);
            return this.InterfaceLogs;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Interface Log entries found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
InterfaceLogListComponent.ctorParameters = () => [
    { type: _interface_log_service__WEBPACK_IMPORTED_MODULE_5__["InterfaceLogService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
InterfaceLogListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-interface-log-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./interface-log-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-log-list/interface-log-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./interface-log-list.component.css */ "./src/app/hardware/interface-log-list/interface-log-list.component.css")).default]
    })
], InterfaceLogListComponent);



/***/ }),

/***/ "./src/app/hardware/interface-log.service.ts":
/*!***************************************************!*\
  !*** ./src/app/hardware/interface-log.service.ts ***!
  \***************************************************/
/*! exports provided: InterfaceLogService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "InterfaceLogService", function() { return InterfaceLogService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let InterfaceLogService = class InterfaceLogService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getInterfaceLogs(InterfaceID) {
        if (typeof (InterfaceID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'InterfaceLogs');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Interfaces/' + InterfaceID + '/InterfaceLogs');
        }
    }
    getInterfaceLog(InterfaceId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'InterfaceLogs'}/${InterfaceId}`);
    }
};
InterfaceLogService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
InterfaceLogService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], InterfaceLogService);



/***/ }),

/***/ "./src/app/hardware/interface-log/interface-log.component.css":
/*!********************************************************************!*\
  !*** ./src/app/hardware/interface-log/interface-log.component.css ***!
  \********************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2hhcmR3YXJlL2ludGVyZmFjZS1sb2cvaW50ZXJmYWNlLWxvZy5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/hardware/interface-log/interface-log.component.ts":
/*!*******************************************************************!*\
  !*** ./src/app/hardware/interface-log/interface-log.component.ts ***!
  \*******************************************************************/
/*! exports provided: InterfaceLogComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "InterfaceLogComponent", function() { return InterfaceLogComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let InterfaceLogComponent = class InterfaceLogComponent {
    constructor() { }
    ngOnInit() {
    }
};
InterfaceLogComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-interface-log',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./interface-log.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface-log/interface-log.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./interface-log.component.css */ "./src/app/hardware/interface-log/interface-log.component.css")).default]
    })
], InterfaceLogComponent);



/***/ }),

/***/ "./src/app/hardware/interface.service.ts":
/*!***********************************************!*\
  !*** ./src/app/hardware/interface.service.ts ***!
  \***********************************************/
/*! exports provided: InterfaceService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "InterfaceService", function() { return InterfaceService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let InterfaceService = class InterfaceService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getInterfaces() {
        return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Interfaces');
    }
    getInterface(InterfaceId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Interfaces'}/${InterfaceId}`);
    }
};
InterfaceService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
InterfaceService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], InterfaceService);



/***/ }),

/***/ "./src/app/hardware/interface/interface.component.css":
/*!************************************************************!*\
  !*** ./src/app/hardware/interface/interface.component.css ***!
  \************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2hhcmR3YXJlL2ludGVyZmFjZS9pbnRlcmZhY2UuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/hardware/interface/interface.component.ts":
/*!***********************************************************!*\
  !*** ./src/app/hardware/interface/interface.component.ts ***!
  \***********************************************************/
/*! exports provided: InterfaceComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "InterfaceComponent", function() { return InterfaceComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let InterfaceComponent = class InterfaceComponent {
    constructor() { }
    ngOnInit() {
    }
};
InterfaceComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-interface',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./interface.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/interface/interface.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./interface.component.css */ "./src/app/hardware/interface/interface.component.css")).default]
    })
], InterfaceComponent);



/***/ }),

/***/ "./src/app/hardware/value-list/value-list.component.css":
/*!**************************************************************!*\
  !*** ./src/app/hardware/value-list/value-list.component.css ***!
  \**************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 125px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaGFyZHdhcmUvdmFsdWUtbGlzdC92YWx1ZS1saXN0LmNvbXBvbmVudC5jc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUE7RUFDRSxXQUFXO0VBQ1gsMkJBQTJCO0FBQzdCOztBQUVBO0VBQ0UsMkJBQTJCO0VBQzNCLGtCQUFrQjtBQUNwQjs7QUFFQTtFQUNFLGNBQWM7RUFDZCxrQkFBa0I7RUFDbEIsZ0JBQWdCO0FBQ2xCIiwiZmlsZSI6InNyYy9hcHAvaGFyZHdhcmUvdmFsdWUtbGlzdC92YWx1ZS1saXN0LmNvbXBvbmVudC5jc3MiLCJzb3VyY2VzQ29udGVudCI6WyI6aG9zdCB7XHJcbiAgd2lkdGg6IDEwMCU7XHJcbiAgbWFyZ2luOiAxMnB4IGF1dG8gYXV0byBhdXRvO1xyXG59XHJcblxyXG4ubWF0LXJvdzpob3ZlciB7XHJcbiAgYmFja2dyb3VuZC1jb2xvcjogbGlnaHRncmF5O1xyXG4gIGJvcmRlci1yYWRpdXM6IDRweDtcclxufVxyXG5cclxuLm1hdC1jYXJkLXN1YnRpdGxlIHtcclxuICBkaXNwbGF5OiBibG9jaztcclxuICBtYXJnaW4tYm90dG9tOiAwcHg7XHJcbiAgbWluLXdpZHRoOiAxMjVweDtcclxufVxyXG4iXX0= */");

/***/ }),

/***/ "./src/app/hardware/value-list/value-list.component.ts":
/*!*************************************************************!*\
  !*** ./src/app/hardware/value-list/value-list.component.ts ***!
  \*************************************************************/
/*! exports provided: ValueListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueListComponent", function() { return ValueListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var src_app_detail_value_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! src/app/detail/value.service */ "./src/app/detail/value.service.ts");






let ValueListComponent = class ValueListComponent {
    constructor(valueService, snackBar, location, route) {
        this.valueService = valueService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.Values = [];
        this.tableColumns = ['ValueID', 'Name', 'DeviceID', 'UnitID', 'Value', 'RetensionDays', 'RetensionInterval', 'Timestamp'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.valueService.getValues(this.route.snapshot.params['id']).subscribe((ValuesMsg) => {
            this.Values = ValuesMsg.Values;
            this.snackBar.open(ValuesMsg.Count + ' Values loaded. ', '', { duration: 3000 });
            console.log(this.Values);
            return this.Values;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Values found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
ValueListComponent.ctorParameters = () => [
    { type: src_app_detail_value_service__WEBPACK_IMPORTED_MODULE_5__["ValueService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
ValueListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-list/value-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-list.component.css */ "./src/app/hardware/value-list/value-list.component.css")).default]
    })
], ValueListComponent);



/***/ }),

/***/ "./src/app/hardware/value-script-list/value-script-list.component.css":
/*!****************************************************************************!*\
  !*** ./src/app/hardware/value-script-list/value-script-list.component.css ***!
  \****************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2hhcmR3YXJlL3ZhbHVlLXNjcmlwdC1saXN0L3ZhbHVlLXNjcmlwdC1saXN0LmNvbXBvbmVudC5jc3MifQ== */");

/***/ }),

/***/ "./src/app/hardware/value-script-list/value-script-list.component.ts":
/*!***************************************************************************!*\
  !*** ./src/app/hardware/value-script-list/value-script-list.component.ts ***!
  \***************************************************************************/
/*! exports provided: ValueScriptListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueScriptListComponent", function() { return ValueScriptListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ValueScriptListComponent = class ValueScriptListComponent {
    constructor() { }
    ngOnInit() {
    }
};
ValueScriptListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-script-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-script-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-script-list/value-script-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-script-list.component.css */ "./src/app/hardware/value-script-list/value-script-list.component.css")).default]
    })
], ValueScriptListComponent);



/***/ }),

/***/ "./src/app/hardware/value-script/value-script.component.css":
/*!******************************************************************!*\
  !*** ./src/app/hardware/value-script/value-script.component.css ***!
  \******************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2hhcmR3YXJlL3ZhbHVlLXNjcmlwdC92YWx1ZS1zY3JpcHQuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/hardware/value-script/value-script.component.ts":
/*!*****************************************************************!*\
  !*** ./src/app/hardware/value-script/value-script.component.ts ***!
  \*****************************************************************/
/*! exports provided: ValueScriptComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueScriptComponent", function() { return ValueScriptComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ValueScriptComponent = class ValueScriptComponent {
    constructor() { }
    ngOnInit() {
    }
};
ValueScriptComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-script',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-script.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/hardware/value-script/value-script.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-script.component.css */ "./src/app/hardware/value-script/value-script.component.css")).default]
    })
], ValueScriptComponent);



/***/ }),

/***/ "./src/app/identity/identity-routing.module.ts":
/*!*****************************************************!*\
  !*** ./src/app/identity/identity-routing.module.ts ***!
  \*****************************************************/
/*! exports provided: IdentityRoutingModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "IdentityRoutingModule", function() { return IdentityRoutingModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var src_app_identity_role_role_component__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! src/app/identity/role/role.component */ "./src/app/identity/role/role.component.ts");
/* harmony import */ var src_app_identity_role_list_role_list_component__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! src/app/identity/role-list/role-list.component */ "./src/app/identity/role-list/role-list.component.ts");
/* harmony import */ var src_app_identity_user_user_component__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! src/app/identity/user/user.component */ "./src/app/identity/user/user.component.ts");
/* harmony import */ var src_app_identity_user_list_user_list_component__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! src/app/identity/user-list/user-list.component */ "./src/app/identity/user-list/user-list.component.ts");
/* harmony import */ var src_app_identity_table_access_table_access_component__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! src/app/identity/table-access/table-access.component */ "./src/app/identity/table-access/table-access.component.ts");
/* harmony import */ var src_app_identity_table_access_list_table_access_list_component__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! src/app/identity/table-access-list/table-access-list.component */ "./src/app/identity/table-access-list/table-access-list.component.ts");









const identityRoutes = [
    { path: 'Roles', component: src_app_identity_role_list_role_list_component__WEBPACK_IMPORTED_MODULE_4__["RoleListComponent"] },
    { path: 'Roles/:id', component: src_app_identity_role_role_component__WEBPACK_IMPORTED_MODULE_3__["RoleComponent"] },
    { path: 'Roles/:id/Users', component: src_app_identity_user_list_user_list_component__WEBPACK_IMPORTED_MODULE_6__["UserListComponent"] },
    { path: 'Roles/:id/TableAccesss', component: src_app_identity_table_access_list_table_access_list_component__WEBPACK_IMPORTED_MODULE_8__["TableAccessListComponent"] },
    { path: 'Users', component: src_app_identity_user_list_user_list_component__WEBPACK_IMPORTED_MODULE_6__["UserListComponent"] },
    { path: 'Users/:id', component: src_app_identity_user_user_component__WEBPACK_IMPORTED_MODULE_5__["UserComponent"] },
    { path: 'TableAccesss/:id', component: src_app_identity_table_access_table_access_component__WEBPACK_IMPORTED_MODULE_7__["TableAccessComponent"] }
];
let IdentityRoutingModule = class IdentityRoutingModule {
};
IdentityRoutingModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        imports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"].forChild(identityRoutes)],
        exports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"]]
    })
], IdentityRoutingModule);



/***/ }),

/***/ "./src/app/identity/identity.module.ts":
/*!*********************************************!*\
  !*** ./src/app/identity/identity.module.ts ***!
  \*********************************************/
/*! exports provided: IdentityModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "IdentityModule", function() { return IdentityModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_forms__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/forms */ "./node_modules/@angular/forms/fesm2015/forms.js");
/* harmony import */ var _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/platform-browser/animations */ "./node_modules/@angular/platform-browser/fesm2015/animations.js");
/* harmony import */ var _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! @angular/material/autocomplete */ "./node_modules/@angular/material/esm2015/autocomplete.js");
/* harmony import */ var _angular_material_badge__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! @angular/material/badge */ "./node_modules/@angular/material/esm2015/badge.js");
/* harmony import */ var _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! @angular/material/bottom-sheet */ "./node_modules/@angular/material/esm2015/bottom-sheet.js");
/* harmony import */ var _angular_material_button__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! @angular/material/button */ "./node_modules/@angular/material/esm2015/button.js");
/* harmony import */ var _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! @angular/material/button-toggle */ "./node_modules/@angular/material/esm2015/button-toggle.js");
/* harmony import */ var _angular_material_card__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! @angular/material/card */ "./node_modules/@angular/material/esm2015/card.js");
/* harmony import */ var _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_11__ = __webpack_require__(/*! @angular/material/checkbox */ "./node_modules/@angular/material/esm2015/checkbox.js");
/* harmony import */ var _angular_material_chips__WEBPACK_IMPORTED_MODULE_12__ = __webpack_require__(/*! @angular/material/chips */ "./node_modules/@angular/material/esm2015/chips.js");
/* harmony import */ var _angular_material_stepper__WEBPACK_IMPORTED_MODULE_13__ = __webpack_require__(/*! @angular/material/stepper */ "./node_modules/@angular/material/esm2015/stepper.js");
/* harmony import */ var _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_14__ = __webpack_require__(/*! @angular/material/datepicker */ "./node_modules/@angular/material/esm2015/datepicker.js");
/* harmony import */ var _angular_material_dialog__WEBPACK_IMPORTED_MODULE_15__ = __webpack_require__(/*! @angular/material/dialog */ "./node_modules/@angular/material/esm2015/dialog.js");
/* harmony import */ var _angular_material_divider__WEBPACK_IMPORTED_MODULE_16__ = __webpack_require__(/*! @angular/material/divider */ "./node_modules/@angular/material/esm2015/divider.js");
/* harmony import */ var _angular_material_expansion__WEBPACK_IMPORTED_MODULE_17__ = __webpack_require__(/*! @angular/material/expansion */ "./node_modules/@angular/material/esm2015/expansion.js");
/* harmony import */ var _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_18__ = __webpack_require__(/*! @angular/material/grid-list */ "./node_modules/@angular/material/esm2015/grid-list.js");
/* harmony import */ var _angular_material_icon__WEBPACK_IMPORTED_MODULE_19__ = __webpack_require__(/*! @angular/material/icon */ "./node_modules/@angular/material/esm2015/icon.js");
/* harmony import */ var _angular_material_input__WEBPACK_IMPORTED_MODULE_20__ = __webpack_require__(/*! @angular/material/input */ "./node_modules/@angular/material/esm2015/input.js");
/* harmony import */ var _angular_material_list__WEBPACK_IMPORTED_MODULE_21__ = __webpack_require__(/*! @angular/material/list */ "./node_modules/@angular/material/esm2015/list.js");
/* harmony import */ var _angular_material_menu__WEBPACK_IMPORTED_MODULE_22__ = __webpack_require__(/*! @angular/material/menu */ "./node_modules/@angular/material/esm2015/menu.js");
/* harmony import */ var _angular_material_core__WEBPACK_IMPORTED_MODULE_23__ = __webpack_require__(/*! @angular/material/core */ "./node_modules/@angular/material/esm2015/core.js");
/* harmony import */ var _angular_material_paginator__WEBPACK_IMPORTED_MODULE_24__ = __webpack_require__(/*! @angular/material/paginator */ "./node_modules/@angular/material/esm2015/paginator.js");
/* harmony import */ var _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_25__ = __webpack_require__(/*! @angular/material/progress-bar */ "./node_modules/@angular/material/esm2015/progress-bar.js");
/* harmony import */ var _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_26__ = __webpack_require__(/*! @angular/material/progress-spinner */ "./node_modules/@angular/material/esm2015/progress-spinner.js");
/* harmony import */ var _angular_material_radio__WEBPACK_IMPORTED_MODULE_27__ = __webpack_require__(/*! @angular/material/radio */ "./node_modules/@angular/material/esm2015/radio.js");
/* harmony import */ var _angular_material_select__WEBPACK_IMPORTED_MODULE_28__ = __webpack_require__(/*! @angular/material/select */ "./node_modules/@angular/material/esm2015/select.js");
/* harmony import */ var _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_29__ = __webpack_require__(/*! @angular/material/sidenav */ "./node_modules/@angular/material/esm2015/sidenav.js");
/* harmony import */ var _angular_material_slider__WEBPACK_IMPORTED_MODULE_30__ = __webpack_require__(/*! @angular/material/slider */ "./node_modules/@angular/material/esm2015/slider.js");
/* harmony import */ var _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_31__ = __webpack_require__(/*! @angular/material/slide-toggle */ "./node_modules/@angular/material/esm2015/slide-toggle.js");
/* harmony import */ var _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_32__ = __webpack_require__(/*! @angular/material/snack-bar */ "./node_modules/@angular/material/esm2015/snack-bar.js");
/* harmony import */ var _angular_material_sort__WEBPACK_IMPORTED_MODULE_33__ = __webpack_require__(/*! @angular/material/sort */ "./node_modules/@angular/material/esm2015/sort.js");
/* harmony import */ var _angular_material_table__WEBPACK_IMPORTED_MODULE_34__ = __webpack_require__(/*! @angular/material/table */ "./node_modules/@angular/material/esm2015/table.js");
/* harmony import */ var _angular_material_tabs__WEBPACK_IMPORTED_MODULE_35__ = __webpack_require__(/*! @angular/material/tabs */ "./node_modules/@angular/material/esm2015/tabs.js");
/* harmony import */ var _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_36__ = __webpack_require__(/*! @angular/material/toolbar */ "./node_modules/@angular/material/esm2015/toolbar.js");
/* harmony import */ var _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_37__ = __webpack_require__(/*! @angular/material/tooltip */ "./node_modules/@angular/material/esm2015/tooltip.js");
/* harmony import */ var _angular_material_tree__WEBPACK_IMPORTED_MODULE_38__ = __webpack_require__(/*! @angular/material/tree */ "./node_modules/@angular/material/esm2015/tree.js");
/* harmony import */ var src_app_identity_role_role_component__WEBPACK_IMPORTED_MODULE_39__ = __webpack_require__(/*! src/app/identity/role/role.component */ "./src/app/identity/role/role.component.ts");
/* harmony import */ var src_app_identity_role_list_role_list_component__WEBPACK_IMPORTED_MODULE_40__ = __webpack_require__(/*! src/app/identity/role-list/role-list.component */ "./src/app/identity/role-list/role-list.component.ts");
/* harmony import */ var src_app_identity_user_user_component__WEBPACK_IMPORTED_MODULE_41__ = __webpack_require__(/*! src/app/identity/user/user.component */ "./src/app/identity/user/user.component.ts");
/* harmony import */ var src_app_identity_user_list_user_list_component__WEBPACK_IMPORTED_MODULE_42__ = __webpack_require__(/*! src/app/identity/user-list/user-list.component */ "./src/app/identity/user-list/user-list.component.ts");
/* harmony import */ var src_app_identity_user_session_user_session_component__WEBPACK_IMPORTED_MODULE_43__ = __webpack_require__(/*! src/app/identity/user-session/user-session.component */ "./src/app/identity/user-session/user-session.component.ts");
/* harmony import */ var src_app_identity_user_session_list_user_session_list_component__WEBPACK_IMPORTED_MODULE_44__ = __webpack_require__(/*! src/app/identity/user-session-list/user-session-list.component */ "./src/app/identity/user-session-list/user-session-list.component.ts");
/* harmony import */ var src_app_identity_table_access_table_access_component__WEBPACK_IMPORTED_MODULE_45__ = __webpack_require__(/*! src/app/identity/table-access/table-access.component */ "./src/app/identity/table-access/table-access.component.ts");
/* harmony import */ var src_app_identity_table_access_list_table_access_list_component__WEBPACK_IMPORTED_MODULE_46__ = __webpack_require__(/*! src/app/identity/table-access-list/table-access-list.component */ "./src/app/identity/table-access-list/table-access-list.component.ts");
/* harmony import */ var src_app_identity_identity_routing_module__WEBPACK_IMPORTED_MODULE_47__ = __webpack_require__(/*! src/app/identity/identity-routing.module */ "./src/app/identity/identity-routing.module.ts");







































// Identity









let IdentityModule = class IdentityModule {
};
IdentityModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        declarations: [
            src_app_identity_role_role_component__WEBPACK_IMPORTED_MODULE_39__["RoleComponent"],
            src_app_identity_role_list_role_list_component__WEBPACK_IMPORTED_MODULE_40__["RoleListComponent"],
            src_app_identity_user_user_component__WEBPACK_IMPORTED_MODULE_41__["UserComponent"],
            src_app_identity_user_list_user_list_component__WEBPACK_IMPORTED_MODULE_42__["UserListComponent"],
            src_app_identity_table_access_table_access_component__WEBPACK_IMPORTED_MODULE_45__["TableAccessComponent"],
            src_app_identity_table_access_list_table_access_list_component__WEBPACK_IMPORTED_MODULE_46__["TableAccessListComponent"],
            src_app_identity_user_session_user_session_component__WEBPACK_IMPORTED_MODULE_43__["UserSessionComponent"],
            src_app_identity_user_session_list_user_session_list_component__WEBPACK_IMPORTED_MODULE_44__["UserSessionListComponent"]
        ],
        imports: [
            _angular_common__WEBPACK_IMPORTED_MODULE_2__["CommonModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["FormsModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["ReactiveFormsModule"],
            _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_4__["BrowserAnimationsModule"],
            _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_5__["MatAutocompleteModule"],
            _angular_material_badge__WEBPACK_IMPORTED_MODULE_6__["MatBadgeModule"],
            _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_7__["MatBottomSheetModule"],
            _angular_material_button__WEBPACK_IMPORTED_MODULE_8__["MatButtonModule"],
            _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_9__["MatButtonToggleModule"],
            _angular_material_card__WEBPACK_IMPORTED_MODULE_10__["MatCardModule"],
            _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_11__["MatCheckboxModule"],
            _angular_material_chips__WEBPACK_IMPORTED_MODULE_12__["MatChipsModule"],
            _angular_material_stepper__WEBPACK_IMPORTED_MODULE_13__["MatStepperModule"],
            _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_14__["MatDatepickerModule"],
            _angular_material_dialog__WEBPACK_IMPORTED_MODULE_15__["MatDialogModule"],
            _angular_material_divider__WEBPACK_IMPORTED_MODULE_16__["MatDividerModule"],
            _angular_material_expansion__WEBPACK_IMPORTED_MODULE_17__["MatExpansionModule"],
            _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_18__["MatGridListModule"],
            _angular_material_icon__WEBPACK_IMPORTED_MODULE_19__["MatIconModule"],
            _angular_material_input__WEBPACK_IMPORTED_MODULE_20__["MatInputModule"],
            _angular_material_list__WEBPACK_IMPORTED_MODULE_21__["MatListModule"],
            _angular_material_menu__WEBPACK_IMPORTED_MODULE_22__["MatMenuModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_23__["MatNativeDateModule"],
            _angular_material_paginator__WEBPACK_IMPORTED_MODULE_24__["MatPaginatorModule"],
            _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_25__["MatProgressBarModule"],
            _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_26__["MatProgressSpinnerModule"],
            _angular_material_radio__WEBPACK_IMPORTED_MODULE_27__["MatRadioModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_23__["MatRippleModule"],
            _angular_material_select__WEBPACK_IMPORTED_MODULE_28__["MatSelectModule"],
            _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_29__["MatSidenavModule"],
            _angular_material_slider__WEBPACK_IMPORTED_MODULE_30__["MatSliderModule"],
            _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_31__["MatSlideToggleModule"],
            _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_32__["MatSnackBarModule"],
            _angular_material_sort__WEBPACK_IMPORTED_MODULE_33__["MatSortModule"],
            _angular_material_table__WEBPACK_IMPORTED_MODULE_34__["MatTableModule"],
            _angular_material_tabs__WEBPACK_IMPORTED_MODULE_35__["MatTabsModule"],
            _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_36__["MatToolbarModule"],
            _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_37__["MatTooltipModule"],
            _angular_material_tree__WEBPACK_IMPORTED_MODULE_38__["MatTreeModule"],
            src_app_identity_identity_routing_module__WEBPACK_IMPORTED_MODULE_47__["IdentityRoutingModule"]
        ]
    })
], IdentityModule);



/***/ }),

/***/ "./src/app/identity/role-list/role-list.component.css":
/*!************************************************************!*\
  !*** ./src/app/identity/role-list/role-list.component.css ***!
  \************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 120px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaWRlbnRpdHkvcm9sZS1saXN0L3JvbGUtbGlzdC5jb21wb25lbnQuY3NzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBO0VBQ0UsV0FBVztFQUNYLDJCQUEyQjtBQUM3Qjs7QUFFQTtFQUNFLDJCQUEyQjtFQUMzQixrQkFBa0I7QUFDcEI7O0FBRUE7RUFDRSxjQUFjO0VBQ2Qsa0JBQWtCO0VBQ2xCLGdCQUFnQjtBQUNsQiIsImZpbGUiOiJzcmMvYXBwL2lkZW50aXR5L3JvbGUtbGlzdC9yb2xlLWxpc3QuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIjpob3N0IHtcclxuICB3aWR0aDogMTAwJTtcclxuICBtYXJnaW46IDEycHggYXV0byBhdXRvIGF1dG87XHJcbn1cclxuXHJcbi5tYXQtcm93OmhvdmVyIHtcclxuICBiYWNrZ3JvdW5kLWNvbG9yOiBsaWdodGdyYXk7XHJcbiAgYm9yZGVyLXJhZGl1czogNHB4O1xyXG59XHJcblxyXG4ubWF0LWNhcmQtc3VidGl0bGUge1xyXG4gIGRpc3BsYXk6IGJsb2NrO1xyXG4gIG1hcmdpbi1ib3R0b206IDBweDtcclxuICBtaW4td2lkdGg6IDEyMHB4O1xyXG59XHJcbiJdfQ== */");

/***/ }),

/***/ "./src/app/identity/role-list/role-list.component.ts":
/*!***********************************************************!*\
  !*** ./src/app/identity/role-list/role-list.component.ts ***!
  \***********************************************************/
/*! exports provided: RoleListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "RoleListComponent", function() { return RoleListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _role_service__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../role.service */ "./src/app/identity/role.service.ts");





let RoleListComponent = class RoleListComponent {
    constructor(roleService, snackBar, location) {
        this.roleService = roleService;
        this.snackBar = snackBar;
        this.location = location;
        this.Roles = [];
        this.tableColumns = ['RoleID', 'Name', 'RemoteAccess'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.roleService.getRoles().subscribe((RolesMsg) => {
            this.Roles = RolesMsg.Roles;
            this.snackBar.open(RolesMsg.Count + ' Roles loaded. ', '', { duration: 3000 });
            console.log(this.Roles);
            return this.Roles;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Roles found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
RoleListComponent.ctorParameters = () => [
    { type: _role_service__WEBPACK_IMPORTED_MODULE_4__["RoleService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_3__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] }
];
RoleListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-role-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./role-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/role-list/role-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./role-list.component.css */ "./src/app/identity/role-list/role-list.component.css")).default]
    })
], RoleListComponent);



/***/ }),

/***/ "./src/app/identity/role.service.ts":
/*!******************************************!*\
  !*** ./src/app/identity/role.service.ts ***!
  \******************************************/
/*! exports provided: RoleService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "RoleService", function() { return RoleService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let RoleService = class RoleService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getRoles() {
        return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Roles');
    }
    getRole(RoleId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Roles'}/${RoleId}`);
    }
};
RoleService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
RoleService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], RoleService);



/***/ }),

/***/ "./src/app/identity/role.ts":
/*!**********************************!*\
  !*** ./src/app/identity/role.ts ***!
  \**********************************/
/*! exports provided: Role, RoleMessage, RolesMessage */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "Role", function() { return Role; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "RoleMessage", function() { return RoleMessage; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "RolesMessage", function() { return RolesMessage; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");

class Role {
    constructor(RoleID, Name, RemoteAccess) {
        this.RoleID = RoleID;
        this.Name = Name;
        this.RemoteAccess = RemoteAccess;
    }
}
class RoleMessage {
}
class RolesMessage {
}


/***/ }),

/***/ "./src/app/identity/role/role.component.css":
/*!**************************************************!*\
  !*** ./src/app/identity/role/role.component.css ***!
  \**************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2lkZW50aXR5L3JvbGUvcm9sZS5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/identity/role/role.component.ts":
/*!*************************************************!*\
  !*** ./src/app/identity/role/role.component.ts ***!
  \*************************************************/
/*! exports provided: RoleComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "RoleComponent", function() { return RoleComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _role_service__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../role.service */ "./src/app/identity/role.service.ts");
/* harmony import */ var _role__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../role */ "./src/app/identity/role.ts");





let RoleComponent = class RoleComponent {
    constructor(RoleService, route) {
        this.RoleService = RoleService;
        this.route = route;
        this.Role = new _role__WEBPACK_IMPORTED_MODULE_4__["Role"](-1, 'No Role', (1 ? true : undefined));
    }
    ngOnInit() {
        debugger;
        this.id = this.route.snapshot.params['id'];
        this.RoleService.getRole(this.id).subscribe((RoleMsg) => {
            this.Role = RoleMsg.Role;
        });
    }
};
RoleComponent.ctorParameters = () => [
    { type: _role_service__WEBPACK_IMPORTED_MODULE_3__["RoleService"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_2__["ActivatedRoute"] }
];
RoleComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-role',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./role.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/role/role.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./role.component.css */ "./src/app/identity/role/role.component.css")).default]
    })
], RoleComponent);



/***/ }),

/***/ "./src/app/identity/table-access-list/table-access-list.component.css":
/*!****************************************************************************!*\
  !*** ./src/app/identity/table-access-list/table-access-list.component.css ***!
  \****************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 170px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaWRlbnRpdHkvdGFibGUtYWNjZXNzLWxpc3QvdGFibGUtYWNjZXNzLWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9pZGVudGl0eS90YWJsZS1hY2Nlc3MtbGlzdC90YWJsZS1hY2Nlc3MtbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTcwcHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/identity/table-access-list/table-access-list.component.ts":
/*!***************************************************************************!*\
  !*** ./src/app/identity/table-access-list/table-access-list.component.ts ***!
  \***************************************************************************/
/*! exports provided: TableAccessListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TableAccessListComponent", function() { return TableAccessListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _table_access_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ../table-access.service */ "./src/app/identity/table-access.service.ts");






let TableAccessListComponent = class TableAccessListComponent {
    constructor(tableAccessService, snackBar, location, route) {
        this.tableAccessService = tableAccessService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.TableAccess = [];
        this.tableColumns = ['TableAccessID', 'Name', 'RoleID', 'CanGET', 'CanPOST', 'CanPUT', 'CanPATCH', 'CanDELETE', 'PUTFields', 'PATCHFields'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.tableAccessService.getTableAccesss(this.route.snapshot.params['id']).subscribe((TableAccessMsg) => {
            this.TableAccess = TableAccessMsg.TableAccesss;
            this.snackBar.open(TableAccessMsg.Count + ' Table Access' + (this.TableAccess.length > 1 ? 'es' : '') + ' loaded. ', '', { duration: 3000 });
            console.log(this.TableAccess);
            return this.TableAccess;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Table Accesses found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
TableAccessListComponent.ctorParameters = () => [
    { type: _table_access_service__WEBPACK_IMPORTED_MODULE_5__["TableAccessService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
TableAccessListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-table-access-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./table-access-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/table-access-list/table-access-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./table-access-list.component.css */ "./src/app/identity/table-access-list/table-access-list.component.css")).default]
    })
], TableAccessListComponent);



/***/ }),

/***/ "./src/app/identity/table-access.service.ts":
/*!**************************************************!*\
  !*** ./src/app/identity/table-access.service.ts ***!
  \**************************************************/
/*! exports provided: TableAccessService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TableAccessService", function() { return TableAccessService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let TableAccessService = class TableAccessService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getTableAccesss(RoleID) {
        if (typeof (RoleID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'TableAccesss');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Roles/' + RoleID + '/TableAccesss');
        }
    }
    getTableAccess(TableAccessId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'TableAccess'}/${TableAccessId}`);
    }
};
TableAccessService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
TableAccessService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], TableAccessService);



/***/ }),

/***/ "./src/app/identity/table-access/table-access.component.css":
/*!******************************************************************!*\
  !*** ./src/app/identity/table-access/table-access.component.css ***!
  \******************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2lkZW50aXR5L3RhYmxlLWFjY2Vzcy90YWJsZS1hY2Nlc3MuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/identity/table-access/table-access.component.ts":
/*!*****************************************************************!*\
  !*** ./src/app/identity/table-access/table-access.component.ts ***!
  \*****************************************************************/
/*! exports provided: TableAccessComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TableAccessComponent", function() { return TableAccessComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let TableAccessComponent = class TableAccessComponent {
    constructor() { }
    ngOnInit() {
    }
};
TableAccessComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-table-access',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./table-access.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/table-access/table-access.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./table-access.component.css */ "./src/app/identity/table-access/table-access.component.css")).default]
    })
], TableAccessComponent);



/***/ }),

/***/ "./src/app/identity/user-list/user-list.component.css":
/*!************************************************************!*\
  !*** ./src/app/identity/user-list/user-list.component.css ***!
  \************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 120px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaWRlbnRpdHkvdXNlci1saXN0L3VzZXItbGlzdC5jb21wb25lbnQuY3NzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBO0VBQ0UsV0FBVztFQUNYLDJCQUEyQjtBQUM3Qjs7QUFFQTtFQUNFLDJCQUEyQjtFQUMzQixrQkFBa0I7QUFDcEI7O0FBRUE7RUFDRSxjQUFjO0VBQ2Qsa0JBQWtCO0VBQ2xCLGdCQUFnQjtBQUNsQiIsImZpbGUiOiJzcmMvYXBwL2lkZW50aXR5L3VzZXItbGlzdC91c2VyLWxpc3QuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIjpob3N0IHtcclxuICB3aWR0aDogMTAwJTtcclxuICBtYXJnaW46IDEycHggYXV0byBhdXRvIGF1dG87XHJcbn1cclxuXHJcbi5tYXQtcm93OmhvdmVyIHtcclxuICBiYWNrZ3JvdW5kLWNvbG9yOiBsaWdodGdyYXk7XHJcbiAgYm9yZGVyLXJhZGl1czogNHB4O1xyXG59XHJcblxyXG4ubWF0LWNhcmQtc3VidGl0bGUge1xyXG4gIGRpc3BsYXk6IGJsb2NrO1xyXG4gIG1hcmdpbi1ib3R0b206IDBweDtcclxuICBtaW4td2lkdGg6IDEyMHB4O1xyXG59XHJcbiJdfQ== */");

/***/ }),

/***/ "./src/app/identity/user-list/user-list.component.ts":
/*!***********************************************************!*\
  !*** ./src/app/identity/user-list/user-list.component.ts ***!
  \***********************************************************/
/*! exports provided: UserListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UserListComponent", function() { return UserListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _user_service__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ../user.service */ "./src/app/identity/user.service.ts");






let UserListComponent = class UserListComponent {
    constructor(userService, snackBar, location, route) {
        this.userService = userService;
        this.snackBar = snackBar;
        this.location = location;
        this.route = route;
        this.Users = [];
        this.tableColumns = ['UserID', 'UserName', 'Password', 'FirstName', 'LastName',
            'RoleID', 'Active', 'ForceChange', 'FailedAttempts', 'Theme', 'Timestamp'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.userService.getUsers(this.route.snapshot.params['id']).subscribe((UsersMsg) => {
            this.Users = UsersMsg.Users;
            this.snackBar.open(UsersMsg.Count + ' User' + (this.Users.length > 1 ? 's' : '') + ' loaded. ', '', { duration: 3000 });
            console.log(this.Users);
            return this.Users;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Users found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
UserListComponent.ctorParameters = () => [
    { type: _user_service__WEBPACK_IMPORTED_MODULE_5__["UserService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_4__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] },
    { type: _angular_router__WEBPACK_IMPORTED_MODULE_3__["ActivatedRoute"] }
];
UserListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-user-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./user-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-list/user-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./user-list.component.css */ "./src/app/identity/user-list/user-list.component.css")).default]
    })
], UserListComponent);



/***/ }),

/***/ "./src/app/identity/user-session-list/user-session-list.component.css":
/*!****************************************************************************!*\
  !*** ./src/app/identity/user-session-list/user-session-list.component.css ***!
  \****************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvaWRlbnRpdHkvdXNlci1zZXNzaW9uLWxpc3QvdXNlci1zZXNzaW9uLWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCIiwiZmlsZSI6InNyYy9hcHAvaWRlbnRpdHkvdXNlci1zZXNzaW9uLWxpc3QvdXNlci1zZXNzaW9uLWxpc3QuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIjpob3N0IHtcclxuICB3aWR0aDogMTAwJTtcclxuICBtYXJnaW46IDEycHggYXV0byBhdXRvIGF1dG87XHJcbn1cclxuXHJcbi5tYXQtcm93OmhvdmVyIHtcclxuICBiYWNrZ3JvdW5kLWNvbG9yOiBsaWdodGdyYXk7XHJcbiAgYm9yZGVyLXJhZGl1czogNHB4O1xyXG59XHJcbiJdfQ== */");

/***/ }),

/***/ "./src/app/identity/user-session-list/user-session-list.component.ts":
/*!***************************************************************************!*\
  !*** ./src/app/identity/user-session-list/user-session-list.component.ts ***!
  \***************************************************************************/
/*! exports provided: UserSessionListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UserSessionListComponent", function() { return UserSessionListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let UserSessionListComponent = class UserSessionListComponent {
    constructor() { }
    ngOnInit() {
    }
};
UserSessionListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-user-session-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./user-session-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-session-list/user-session-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./user-session-list.component.css */ "./src/app/identity/user-session-list/user-session-list.component.css")).default]
    })
], UserSessionListComponent);



/***/ }),

/***/ "./src/app/identity/user-session/user-session.component.css":
/*!******************************************************************!*\
  !*** ./src/app/identity/user-session/user-session.component.css ***!
  \******************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2lkZW50aXR5L3VzZXItc2Vzc2lvbi91c2VyLXNlc3Npb24uY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/identity/user-session/user-session.component.ts":
/*!*****************************************************************!*\
  !*** ./src/app/identity/user-session/user-session.component.ts ***!
  \*****************************************************************/
/*! exports provided: UserSessionComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UserSessionComponent", function() { return UserSessionComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let UserSessionComponent = class UserSessionComponent {
    constructor() { }
    ngOnInit() {
    }
};
UserSessionComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-user-session',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./user-session.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user-session/user-session.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./user-session.component.css */ "./src/app/identity/user-session/user-session.component.css")).default]
    })
], UserSessionComponent);



/***/ }),

/***/ "./src/app/identity/user.service.ts":
/*!******************************************!*\
  !*** ./src/app/identity/user.service.ts ***!
  \******************************************/
/*! exports provided: UserService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UserService", function() { return UserService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let UserService = class UserService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getUsers(RoleID) {
        if (typeof (RoleID) == 'undefined') {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Users');
        }
        else {
            return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Roles/' + RoleID + '/Users');
        }
    }
    getUser(UserId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Users'}/${UserId}`);
    }
};
UserService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
UserService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], UserService);



/***/ }),

/***/ "./src/app/identity/user/user.component.css":
/*!**************************************************!*\
  !*** ./src/app/identity/user/user.component.css ***!
  \**************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL2lkZW50aXR5L3VzZXIvdXNlci5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/identity/user/user.component.ts":
/*!*************************************************!*\
  !*** ./src/app/identity/user/user.component.ts ***!
  \*************************************************/
/*! exports provided: UserComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UserComponent", function() { return UserComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let UserComponent = class UserComponent {
    constructor() { }
    ngOnInit() {
    }
};
UserComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-user',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./user.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/identity/user/user.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./user.component.css */ "./src/app/identity/user/user.component.css")).default]
    })
], UserComponent);



/***/ }),

/***/ "./src/app/system/preference-list/preference-list.component.css":
/*!**********************************************************************!*\
  !*** ./src/app/system/preference-list/preference-list.component.css ***!
  \**********************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 155px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3ByZWZlcmVuY2UtbGlzdC9wcmVmZXJlbmNlLWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9zeXN0ZW0vcHJlZmVyZW5jZS1saXN0L3ByZWZlcmVuY2UtbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTU1cHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/system/preference-list/preference-list.component.ts":
/*!*********************************************************************!*\
  !*** ./src/app/system/preference-list/preference-list.component.ts ***!
  \*********************************************************************/
/*! exports provided: PreferenceListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "PreferenceListComponent", function() { return PreferenceListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _preference_service__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../preference.service */ "./src/app/system/preference.service.ts");





let PreferenceListComponent = class PreferenceListComponent {
    constructor(PreferenceService, snackBar, location) {
        this.PreferenceService = PreferenceService;
        this.snackBar = snackBar;
        this.location = location;
        this.Preferences = [];
        this.tableColumns = ['PreferenceID', 'Name', 'Value'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.PreferenceService.getPreferences().subscribe((PreferencesMsg) => {
            this.Preferences = PreferencesMsg.Preferences;
            console.log(this.Preferences);
            this.snackBar.open(PreferencesMsg.Count + ' Preference' + (this.Preferences.length > 1 ? 's' : '') + ' loaded. ', '', { duration: 3000 });
            return this.Preferences;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Preferences found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
PreferenceListComponent.ctorParameters = () => [
    { type: _preference_service__WEBPACK_IMPORTED_MODULE_4__["PreferenceService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_3__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] }
];
PreferenceListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-preference-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./preference-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/preference-list/preference-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./preference-list.component.css */ "./src/app/system/preference-list/preference-list.component.css")).default]
    })
], PreferenceListComponent);



/***/ }),

/***/ "./src/app/system/preference.service.ts":
/*!**********************************************!*\
  !*** ./src/app/system/preference.service.ts ***!
  \**********************************************/
/*! exports provided: PreferenceService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "PreferenceService", function() { return PreferenceService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let PreferenceService = class PreferenceService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getPreferences() {
        return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Preferences');
    }
    getPreference(PreferenceId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Preferences'}/${PreferenceId}`);
    }
};
PreferenceService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
PreferenceService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], PreferenceService);



/***/ }),

/***/ "./src/app/system/preference.ts":
/*!**************************************!*\
  !*** ./src/app/system/preference.ts ***!
  \**************************************/
/*! exports provided: Preference, PreferenceMessage, PreferencesMessage */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "Preference", function() { return Preference; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "PreferenceMessage", function() { return PreferenceMessage; });
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "PreferencesMessage", function() { return PreferencesMessage; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");

class Preference {
    constructor(PreferenceID, Name, Value) {
        this.PreferenceID = PreferenceID;
        this.Name = Name;
        this.Value = Value;
    }
}
class PreferenceMessage {
}
class PreferencesMessage {
}


/***/ }),

/***/ "./src/app/system/preference/preference.component.css":
/*!************************************************************!*\
  !*** ./src/app/system/preference/preference.component.css ***!
  \************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS9wcmVmZXJlbmNlL3ByZWZlcmVuY2UuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/system/preference/preference.component.ts":
/*!***********************************************************!*\
  !*** ./src/app/system/preference/preference.component.ts ***!
  \***********************************************************/
/*! exports provided: PreferenceComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "PreferenceComponent", function() { return PreferenceComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _preference_service__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! ../preference.service */ "./src/app/system/preference.service.ts");
/* harmony import */ var _preference__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../preference */ "./src/app/system/preference.ts");




let PreferenceComponent = class PreferenceComponent {
    constructor(PreferenceService) {
        this.PreferenceService = PreferenceService;
        this.Preference = new _preference__WEBPACK_IMPORTED_MODULE_3__["Preference"](-1, 'No Preference', "No Value");
    }
    ngOnInit() {
        this.PreferenceService.getPreference(1).subscribe((PreferenceMsg) => {
            this.Preference = PreferenceMsg.Preference;
        });
    }
};
PreferenceComponent.ctorParameters = () => [
    { type: _preference_service__WEBPACK_IMPORTED_MODULE_2__["PreferenceService"] }
];
PreferenceComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-preference',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./preference.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/preference/preference.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./preference.component.css */ "./src/app/system/preference/preference.component.css")).default]
    })
], PreferenceComponent);



/***/ }),

/***/ "./src/app/system/scene-list/scene-list.component.css":
/*!************************************************************!*\
  !*** ./src/app/system/scene-list/scene-list.component.css ***!
  \************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 130px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3NjZW5lLWxpc3Qvc2NlbmUtbGlzdC5jb21wb25lbnQuY3NzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBO0VBQ0UsV0FBVztFQUNYLDJCQUEyQjtBQUM3Qjs7QUFFQTtFQUNFLDJCQUEyQjtFQUMzQixrQkFBa0I7QUFDcEI7O0FBRUE7RUFDRSxjQUFjO0VBQ2Qsa0JBQWtCO0VBQ2xCLGdCQUFnQjtBQUNsQiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS9zY2VuZS1saXN0L3NjZW5lLWxpc3QuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIjpob3N0IHtcclxuICB3aWR0aDogMTAwJTtcclxuICBtYXJnaW46IDEycHggYXV0byBhdXRvIGF1dG87XHJcbn1cclxuXHJcbi5tYXQtcm93OmhvdmVyIHtcclxuICBiYWNrZ3JvdW5kLWNvbG9yOiBsaWdodGdyYXk7XHJcbiAgYm9yZGVyLXJhZGl1czogNHB4O1xyXG59XHJcblxyXG4ubWF0LWNhcmQtc3VidGl0bGUge1xyXG4gIGRpc3BsYXk6IGJsb2NrO1xyXG4gIG1hcmdpbi1ib3R0b206IDBweDtcclxuICBtaW4td2lkdGg6IDEzMHB4O1xyXG59XHJcbiJdfQ== */");

/***/ }),

/***/ "./src/app/system/scene-list/scene-list.component.ts":
/*!***********************************************************!*\
  !*** ./src/app/system/scene-list/scene-list.component.ts ***!
  \***********************************************************/
/*! exports provided: SceneListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SceneListComponent", function() { return SceneListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _scene_service__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../scene.service */ "./src/app/system/scene.service.ts");





let SceneListComponent = class SceneListComponent {
    constructor(sceneService, snackBar, location) {
        this.sceneService = sceneService;
        this.snackBar = snackBar;
        this.location = location;
        this.Scenes = [];
        this.tableColumns = ['SceneID', 'Name', 'Active'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.sceneService.getScenes().subscribe((ScenesMsg) => {
            this.Scenes = ScenesMsg.Scenes;
            this.snackBar.open(ScenesMsg.Count + ' Scene' + (this.Scenes.length > 1 ? 's' : '') + ' loaded. ', '', { duration: 3000 });
            console.log(this.Scenes);
            return this.Scenes;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Scenes found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
SceneListComponent.ctorParameters = () => [
    { type: _scene_service__WEBPACK_IMPORTED_MODULE_4__["SceneService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_3__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] }
];
SceneListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-scene-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./scene-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-list/scene-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./scene-list.component.css */ "./src/app/system/scene-list/scene-list.component.css")).default]
    })
], SceneListComponent);



/***/ }),

/***/ "./src/app/system/scene-value-list/scene-value-list.component.css":
/*!************************************************************************!*\
  !*** ./src/app/system/scene-value-list/scene-value-list.component.css ***!
  \************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3NjZW5lLXZhbHVlLWxpc3Qvc2NlbmUtdmFsdWUtbGlzdC5jb21wb25lbnQuY3NzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBO0VBQ0UsV0FBVztFQUNYLDJCQUEyQjtBQUM3Qjs7QUFFQTtFQUNFLDJCQUEyQjtFQUMzQixrQkFBa0I7QUFDcEIiLCJmaWxlIjoic3JjL2FwcC9zeXN0ZW0vc2NlbmUtdmFsdWUtbGlzdC9zY2VuZS12YWx1ZS1saXN0LmNvbXBvbmVudC5jc3MiLCJzb3VyY2VzQ29udGVudCI6WyI6aG9zdCB7XHJcbiAgd2lkdGg6IDEwMCU7XHJcbiAgbWFyZ2luOiAxMnB4IGF1dG8gYXV0byBhdXRvO1xyXG59XHJcblxyXG4ubWF0LXJvdzpob3ZlciB7XHJcbiAgYmFja2dyb3VuZC1jb2xvcjogbGlnaHRncmF5O1xyXG4gIGJvcmRlci1yYWRpdXM6IDRweDtcclxufVxyXG4iXX0= */");

/***/ }),

/***/ "./src/app/system/scene-value-list/scene-value-list.component.ts":
/*!***********************************************************************!*\
  !*** ./src/app/system/scene-value-list/scene-value-list.component.ts ***!
  \***********************************************************************/
/*! exports provided: SceneValueListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SceneValueListComponent", function() { return SceneValueListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let SceneValueListComponent = class SceneValueListComponent {
    constructor() { }
    ngOnInit() {
    }
};
SceneValueListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-scene-value-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./scene-value-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-value-list/scene-value-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./scene-value-list.component.css */ "./src/app/system/scene-value-list/scene-value-list.component.css")).default]
    })
], SceneValueListComponent);



/***/ }),

/***/ "./src/app/system/scene-value/scene-value.component.css":
/*!**************************************************************!*\
  !*** ./src/app/system/scene-value/scene-value.component.css ***!
  \**************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (".ng-star-inserted {\r\n  width: 100%;\r\n  padding: 120px;\r\n  margin: 120px 12px auto auto;\r\n  align-content: center;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3NjZW5lLXZhbHVlL3NjZW5lLXZhbHVlLmNvbXBvbmVudC5jc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUE7RUFDRSxXQUFXO0VBQ1gsY0FBYztFQUNkLDRCQUE0QjtFQUM1QixxQkFBcUI7QUFDdkIiLCJmaWxlIjoic3JjL2FwcC9zeXN0ZW0vc2NlbmUtdmFsdWUvc2NlbmUtdmFsdWUuY29tcG9uZW50LmNzcyIsInNvdXJjZXNDb250ZW50IjpbIi5uZy1zdGFyLWluc2VydGVkIHtcclxuICB3aWR0aDogMTAwJTtcclxuICBwYWRkaW5nOiAxMjBweDtcclxuICBtYXJnaW46IDEyMHB4IDEycHggYXV0byBhdXRvO1xyXG4gIGFsaWduLWNvbnRlbnQ6IGNlbnRlcjtcclxufVxyXG4iXX0= */");

/***/ }),

/***/ "./src/app/system/scene-value/scene-value.component.ts":
/*!*************************************************************!*\
  !*** ./src/app/system/scene-value/scene-value.component.ts ***!
  \*************************************************************/
/*! exports provided: SceneValueComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SceneValueComponent", function() { return SceneValueComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let SceneValueComponent = class SceneValueComponent {
    constructor() { }
    ngOnInit() {
    }
};
SceneValueComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-scene-value',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./scene-value.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene-value/scene-value.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./scene-value.component.css */ "./src/app/system/scene-value/scene-value.component.css")).default]
    })
], SceneValueComponent);



/***/ }),

/***/ "./src/app/system/scene.service.ts":
/*!*****************************************!*\
  !*** ./src/app/system/scene.service.ts ***!
  \*****************************************/
/*! exports provided: SceneService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SceneService", function() { return SceneService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let SceneService = class SceneService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getScenes() {
        return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Scenes');
    }
    getScene(SceneId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Scenes'}/${SceneId}`);
    }
};
SceneService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
SceneService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], SceneService);



/***/ }),

/***/ "./src/app/system/scene/scene.component.css":
/*!**************************************************!*\
  !*** ./src/app/system/scene/scene.component.css ***!
  \**************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS9zY2VuZS9zY2VuZS5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/system/scene/scene.component.ts":
/*!*************************************************!*\
  !*** ./src/app/system/scene/scene.component.ts ***!
  \*************************************************/
/*! exports provided: SceneComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SceneComponent", function() { return SceneComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let SceneComponent = class SceneComponent {
    constructor() { }
    ngOnInit() {
    }
};
SceneComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-scene',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./scene.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/scene/scene.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./scene.component.css */ "./src/app/system/scene/scene.component.css")).default]
    })
], SceneComponent);



/***/ }),

/***/ "./src/app/system/standard-script-list/standard-script-list.component.css":
/*!********************************************************************************!*\
  !*** ./src/app/system/standard-script-list/standard-script-list.component.css ***!
  \********************************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3N0YW5kYXJkLXNjcmlwdC1saXN0L3N0YW5kYXJkLXNjcmlwdC1saXN0LmNvbXBvbmVudC5jc3MiXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IkFBQUE7RUFDRSxXQUFXO0VBQ1gsMkJBQTJCO0FBQzdCOztBQUVBO0VBQ0UsMkJBQTJCO0VBQzNCLGtCQUFrQjtBQUNwQiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS9zdGFuZGFyZC1zY3JpcHQtbGlzdC9zdGFuZGFyZC1zY3JpcHQtbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/system/standard-script-list/standard-script-list.component.ts":
/*!*******************************************************************************!*\
  !*** ./src/app/system/standard-script-list/standard-script-list.component.ts ***!
  \*******************************************************************************/
/*! exports provided: StandardScriptListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "StandardScriptListComponent", function() { return StandardScriptListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let StandardScriptListComponent = class StandardScriptListComponent {
    constructor() { }
    ngOnInit() {
    }
};
StandardScriptListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-standard-script-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./standard-script-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/standard-script-list/standard-script-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./standard-script-list.component.css */ "./src/app/system/standard-script-list/standard-script-list.component.css")).default]
    })
], StandardScriptListComponent);



/***/ }),

/***/ "./src/app/system/standard-script/standard-script.component.css":
/*!**********************************************************************!*\
  !*** ./src/app/system/standard-script/standard-script.component.css ***!
  \**********************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS9zdGFuZGFyZC1zY3JpcHQvc3RhbmRhcmQtc2NyaXB0LmNvbXBvbmVudC5jc3MifQ== */");

/***/ }),

/***/ "./src/app/system/standard-script/standard-script.component.ts":
/*!*********************************************************************!*\
  !*** ./src/app/system/standard-script/standard-script.component.ts ***!
  \*********************************************************************/
/*! exports provided: StandardScriptComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "StandardScriptComponent", function() { return StandardScriptComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let StandardScriptComponent = class StandardScriptComponent {
    constructor() { }
    ngOnInit() {
    }
};
StandardScriptComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-standard-script',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./standard-script.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/standard-script/standard-script.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./standard-script.component.css */ "./src/app/system/standard-script/standard-script.component.css")).default]
    })
], StandardScriptComponent);



/***/ }),

/***/ "./src/app/system/system-routing.module.ts":
/*!*************************************************!*\
  !*** ./src/app/system/system-routing.module.ts ***!
  \*************************************************/
/*! exports provided: SystemRoutingModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SystemRoutingModule", function() { return SystemRoutingModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_router__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/router */ "./node_modules/@angular/router/fesm2015/router.js");
/* harmony import */ var src_app_system_preference_preference_component__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! src/app/system/preference/preference.component */ "./src/app/system/preference/preference.component.ts");
/* harmony import */ var src_app_system_preference_list_preference_list_component__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! src/app/system/preference-list/preference-list.component */ "./src/app/system/preference-list/preference-list.component.ts");
/* harmony import */ var src_app_system_timer_plan_timer_plan_component__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! src/app/system/timer-plan/timer-plan.component */ "./src/app/system/timer-plan/timer-plan.component.ts");
/* harmony import */ var src_app_system_timer_plan_list_timer_plan_list_component__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! src/app/system/timer-plan-list/timer-plan-list.component */ "./src/app/system/timer-plan-list/timer-plan-list.component.ts");
/* harmony import */ var src_app_system_unit_unit_component__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! src/app/system/unit/unit.component */ "./src/app/system/unit/unit.component.ts");
/* harmony import */ var src_app_system_unit_list_unit_list_component__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! src/app/system/unit-list/unit-list.component */ "./src/app/system/unit-list/unit-list.component.ts");
/* harmony import */ var src_app_system_scene_scene_component__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! src/app/system/scene/scene.component */ "./src/app/system/scene/scene.component.ts");
/* harmony import */ var src_app_system_scene_list_scene_list_component__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! src/app/system/scene-list/scene-list.component */ "./src/app/system/scene-list/scene-list.component.ts");











const systemRoutes = [
    { path: 'Preferences', component: src_app_system_preference_list_preference_list_component__WEBPACK_IMPORTED_MODULE_4__["PreferenceListComponent"] },
    { path: 'Preferences/:id', component: src_app_system_preference_preference_component__WEBPACK_IMPORTED_MODULE_3__["PreferenceComponent"] },
    { path: 'TimerPlans', component: src_app_system_timer_plan_list_timer_plan_list_component__WEBPACK_IMPORTED_MODULE_6__["TimerPlanListComponent"] },
    { path: 'TimerPlans/:id', component: src_app_system_timer_plan_timer_plan_component__WEBPACK_IMPORTED_MODULE_5__["TimerPlanComponent"] },
    { path: 'Units', component: src_app_system_unit_list_unit_list_component__WEBPACK_IMPORTED_MODULE_8__["UnitListComponent"] },
    { path: 'Units/:id', component: src_app_system_unit_unit_component__WEBPACK_IMPORTED_MODULE_7__["UnitComponent"] },
    { path: 'Scenes', component: src_app_system_scene_list_scene_list_component__WEBPACK_IMPORTED_MODULE_10__["SceneListComponent"] },
    { path: 'Scenes/:id', component: src_app_system_scene_scene_component__WEBPACK_IMPORTED_MODULE_9__["SceneComponent"] }
];
let SystemRoutingModule = class SystemRoutingModule {
};
SystemRoutingModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        imports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"].forChild(systemRoutes)],
        exports: [_angular_router__WEBPACK_IMPORTED_MODULE_2__["RouterModule"]]
    })
], SystemRoutingModule);



/***/ }),

/***/ "./src/app/system/system.module.ts":
/*!*****************************************!*\
  !*** ./src/app/system/system.module.ts ***!
  \*****************************************/
/*! exports provided: SystemModule */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "SystemModule", function() { return SystemModule; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_forms__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/forms */ "./node_modules/@angular/forms/fesm2015/forms.js");
/* harmony import */ var _angular_platform_browser__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! @angular/platform-browser */ "./node_modules/@angular/platform-browser/fesm2015/platform-browser.js");
/* harmony import */ var _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! @angular/platform-browser/animations */ "./node_modules/@angular/platform-browser/fesm2015/animations.js");
/* harmony import */ var _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_6__ = __webpack_require__(/*! @angular/material/autocomplete */ "./node_modules/@angular/material/esm2015/autocomplete.js");
/* harmony import */ var _angular_material_badge__WEBPACK_IMPORTED_MODULE_7__ = __webpack_require__(/*! @angular/material/badge */ "./node_modules/@angular/material/esm2015/badge.js");
/* harmony import */ var _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_8__ = __webpack_require__(/*! @angular/material/bottom-sheet */ "./node_modules/@angular/material/esm2015/bottom-sheet.js");
/* harmony import */ var _angular_material_button__WEBPACK_IMPORTED_MODULE_9__ = __webpack_require__(/*! @angular/material/button */ "./node_modules/@angular/material/esm2015/button.js");
/* harmony import */ var _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_10__ = __webpack_require__(/*! @angular/material/button-toggle */ "./node_modules/@angular/material/esm2015/button-toggle.js");
/* harmony import */ var _angular_material_card__WEBPACK_IMPORTED_MODULE_11__ = __webpack_require__(/*! @angular/material/card */ "./node_modules/@angular/material/esm2015/card.js");
/* harmony import */ var _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_12__ = __webpack_require__(/*! @angular/material/checkbox */ "./node_modules/@angular/material/esm2015/checkbox.js");
/* harmony import */ var _angular_material_chips__WEBPACK_IMPORTED_MODULE_13__ = __webpack_require__(/*! @angular/material/chips */ "./node_modules/@angular/material/esm2015/chips.js");
/* harmony import */ var _angular_material_stepper__WEBPACK_IMPORTED_MODULE_14__ = __webpack_require__(/*! @angular/material/stepper */ "./node_modules/@angular/material/esm2015/stepper.js");
/* harmony import */ var _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_15__ = __webpack_require__(/*! @angular/material/datepicker */ "./node_modules/@angular/material/esm2015/datepicker.js");
/* harmony import */ var _angular_material_dialog__WEBPACK_IMPORTED_MODULE_16__ = __webpack_require__(/*! @angular/material/dialog */ "./node_modules/@angular/material/esm2015/dialog.js");
/* harmony import */ var _angular_material_divider__WEBPACK_IMPORTED_MODULE_17__ = __webpack_require__(/*! @angular/material/divider */ "./node_modules/@angular/material/esm2015/divider.js");
/* harmony import */ var _angular_material_expansion__WEBPACK_IMPORTED_MODULE_18__ = __webpack_require__(/*! @angular/material/expansion */ "./node_modules/@angular/material/esm2015/expansion.js");
/* harmony import */ var _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_19__ = __webpack_require__(/*! @angular/material/grid-list */ "./node_modules/@angular/material/esm2015/grid-list.js");
/* harmony import */ var _angular_material_icon__WEBPACK_IMPORTED_MODULE_20__ = __webpack_require__(/*! @angular/material/icon */ "./node_modules/@angular/material/esm2015/icon.js");
/* harmony import */ var _angular_material_input__WEBPACK_IMPORTED_MODULE_21__ = __webpack_require__(/*! @angular/material/input */ "./node_modules/@angular/material/esm2015/input.js");
/* harmony import */ var _angular_material_list__WEBPACK_IMPORTED_MODULE_22__ = __webpack_require__(/*! @angular/material/list */ "./node_modules/@angular/material/esm2015/list.js");
/* harmony import */ var _angular_material_menu__WEBPACK_IMPORTED_MODULE_23__ = __webpack_require__(/*! @angular/material/menu */ "./node_modules/@angular/material/esm2015/menu.js");
/* harmony import */ var _angular_material_core__WEBPACK_IMPORTED_MODULE_24__ = __webpack_require__(/*! @angular/material/core */ "./node_modules/@angular/material/esm2015/core.js");
/* harmony import */ var _angular_material_paginator__WEBPACK_IMPORTED_MODULE_25__ = __webpack_require__(/*! @angular/material/paginator */ "./node_modules/@angular/material/esm2015/paginator.js");
/* harmony import */ var _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_26__ = __webpack_require__(/*! @angular/material/progress-bar */ "./node_modules/@angular/material/esm2015/progress-bar.js");
/* harmony import */ var _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_27__ = __webpack_require__(/*! @angular/material/progress-spinner */ "./node_modules/@angular/material/esm2015/progress-spinner.js");
/* harmony import */ var _angular_material_radio__WEBPACK_IMPORTED_MODULE_28__ = __webpack_require__(/*! @angular/material/radio */ "./node_modules/@angular/material/esm2015/radio.js");
/* harmony import */ var _angular_material_select__WEBPACK_IMPORTED_MODULE_29__ = __webpack_require__(/*! @angular/material/select */ "./node_modules/@angular/material/esm2015/select.js");
/* harmony import */ var _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_30__ = __webpack_require__(/*! @angular/material/sidenav */ "./node_modules/@angular/material/esm2015/sidenav.js");
/* harmony import */ var _angular_material_slider__WEBPACK_IMPORTED_MODULE_31__ = __webpack_require__(/*! @angular/material/slider */ "./node_modules/@angular/material/esm2015/slider.js");
/* harmony import */ var _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_32__ = __webpack_require__(/*! @angular/material/slide-toggle */ "./node_modules/@angular/material/esm2015/slide-toggle.js");
/* harmony import */ var _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_33__ = __webpack_require__(/*! @angular/material/snack-bar */ "./node_modules/@angular/material/esm2015/snack-bar.js");
/* harmony import */ var _angular_material_sort__WEBPACK_IMPORTED_MODULE_34__ = __webpack_require__(/*! @angular/material/sort */ "./node_modules/@angular/material/esm2015/sort.js");
/* harmony import */ var _angular_material_table__WEBPACK_IMPORTED_MODULE_35__ = __webpack_require__(/*! @angular/material/table */ "./node_modules/@angular/material/esm2015/table.js");
/* harmony import */ var _angular_material_tabs__WEBPACK_IMPORTED_MODULE_36__ = __webpack_require__(/*! @angular/material/tabs */ "./node_modules/@angular/material/esm2015/tabs.js");
/* harmony import */ var _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_37__ = __webpack_require__(/*! @angular/material/toolbar */ "./node_modules/@angular/material/esm2015/toolbar.js");
/* harmony import */ var _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_38__ = __webpack_require__(/*! @angular/material/tooltip */ "./node_modules/@angular/material/esm2015/tooltip.js");
/* harmony import */ var _angular_material_tree__WEBPACK_IMPORTED_MODULE_39__ = __webpack_require__(/*! @angular/material/tree */ "./node_modules/@angular/material/esm2015/tree.js");
/* harmony import */ var src_app_system_preference_preference_component__WEBPACK_IMPORTED_MODULE_40__ = __webpack_require__(/*! src/app/system/preference/preference.component */ "./src/app/system/preference/preference.component.ts");
/* harmony import */ var src_app_system_preference_list_preference_list_component__WEBPACK_IMPORTED_MODULE_41__ = __webpack_require__(/*! src/app/system/preference-list/preference-list.component */ "./src/app/system/preference-list/preference-list.component.ts");
/* harmony import */ var src_app_system_unit_unit_component__WEBPACK_IMPORTED_MODULE_42__ = __webpack_require__(/*! src/app/system/unit/unit.component */ "./src/app/system/unit/unit.component.ts");
/* harmony import */ var src_app_system_unit_list_unit_list_component__WEBPACK_IMPORTED_MODULE_43__ = __webpack_require__(/*! src/app/system/unit-list/unit-list.component */ "./src/app/system/unit-list/unit-list.component.ts");
/* harmony import */ var src_app_system_standard_script_standard_script_component__WEBPACK_IMPORTED_MODULE_44__ = __webpack_require__(/*! src/app/system/standard-script/standard-script.component */ "./src/app/system/standard-script/standard-script.component.ts");
/* harmony import */ var src_app_system_standard_script_list_standard_script_list_component__WEBPACK_IMPORTED_MODULE_45__ = __webpack_require__(/*! src/app/system/standard-script-list/standard-script-list.component */ "./src/app/system/standard-script-list/standard-script-list.component.ts");
/* harmony import */ var src_app_system_timer_plan_timer_plan_component__WEBPACK_IMPORTED_MODULE_46__ = __webpack_require__(/*! src/app/system/timer-plan/timer-plan.component */ "./src/app/system/timer-plan/timer-plan.component.ts");
/* harmony import */ var src_app_system_timer_plan_list_timer_plan_list_component__WEBPACK_IMPORTED_MODULE_47__ = __webpack_require__(/*! src/app/system/timer-plan-list/timer-plan-list.component */ "./src/app/system/timer-plan-list/timer-plan-list.component.ts");
/* harmony import */ var src_app_system_value_timer_value_timer_component__WEBPACK_IMPORTED_MODULE_48__ = __webpack_require__(/*! src/app/system/value-timer/value-timer.component */ "./src/app/system/value-timer/value-timer.component.ts");
/* harmony import */ var src_app_system_scene_scene_component__WEBPACK_IMPORTED_MODULE_49__ = __webpack_require__(/*! src/app/system/scene/scene.component */ "./src/app/system/scene/scene.component.ts");
/* harmony import */ var src_app_system_scene_list_scene_list_component__WEBPACK_IMPORTED_MODULE_50__ = __webpack_require__(/*! src/app/system/scene-list/scene-list.component */ "./src/app/system/scene-list/scene-list.component.ts");
/* harmony import */ var src_app_system_scene_value_scene_value_component__WEBPACK_IMPORTED_MODULE_51__ = __webpack_require__(/*! src/app/system/scene-value/scene-value.component */ "./src/app/system/scene-value/scene-value.component.ts");
/* harmony import */ var src_app_system_scene_value_list_scene_value_list_component__WEBPACK_IMPORTED_MODULE_52__ = __webpack_require__(/*! src/app/system/scene-value-list/scene-value-list.component */ "./src/app/system/scene-value-list/scene-value-list.component.ts");
/* harmony import */ var src_app_system_system_routing_module__WEBPACK_IMPORTED_MODULE_53__ = __webpack_require__(/*! src/app/system/system-routing.module */ "./src/app/system/system-routing.module.ts");








































// System














let SystemModule = class SystemModule {
};
SystemModule = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["NgModule"])({
        declarations: [
            src_app_system_standard_script_standard_script_component__WEBPACK_IMPORTED_MODULE_44__["StandardScriptComponent"],
            src_app_system_standard_script_list_standard_script_list_component__WEBPACK_IMPORTED_MODULE_45__["StandardScriptListComponent"],
            src_app_system_scene_scene_component__WEBPACK_IMPORTED_MODULE_49__["SceneComponent"],
            src_app_system_scene_list_scene_list_component__WEBPACK_IMPORTED_MODULE_50__["SceneListComponent"],
            src_app_system_scene_value_scene_value_component__WEBPACK_IMPORTED_MODULE_51__["SceneValueComponent"],
            src_app_system_scene_value_list_scene_value_list_component__WEBPACK_IMPORTED_MODULE_52__["SceneValueListComponent"],
            src_app_system_preference_preference_component__WEBPACK_IMPORTED_MODULE_40__["PreferenceComponent"],
            src_app_system_preference_list_preference_list_component__WEBPACK_IMPORTED_MODULE_41__["PreferenceListComponent"],
            src_app_system_timer_plan_timer_plan_component__WEBPACK_IMPORTED_MODULE_46__["TimerPlanComponent"],
            src_app_system_timer_plan_list_timer_plan_list_component__WEBPACK_IMPORTED_MODULE_47__["TimerPlanListComponent"],
            src_app_system_unit_unit_component__WEBPACK_IMPORTED_MODULE_42__["UnitComponent"],
            src_app_system_unit_list_unit_list_component__WEBPACK_IMPORTED_MODULE_43__["UnitListComponent"],
            src_app_system_value_timer_value_timer_component__WEBPACK_IMPORTED_MODULE_48__["ValueTimerComponent"]
        ],
        imports: [
            _angular_common__WEBPACK_IMPORTED_MODULE_2__["CommonModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["FormsModule"],
            _angular_forms__WEBPACK_IMPORTED_MODULE_3__["ReactiveFormsModule"],
            _angular_platform_browser__WEBPACK_IMPORTED_MODULE_4__["BrowserModule"],
            _angular_platform_browser_animations__WEBPACK_IMPORTED_MODULE_5__["BrowserAnimationsModule"],
            _angular_material_autocomplete__WEBPACK_IMPORTED_MODULE_6__["MatAutocompleteModule"],
            _angular_material_badge__WEBPACK_IMPORTED_MODULE_7__["MatBadgeModule"],
            _angular_material_bottom_sheet__WEBPACK_IMPORTED_MODULE_8__["MatBottomSheetModule"],
            _angular_material_button__WEBPACK_IMPORTED_MODULE_9__["MatButtonModule"],
            _angular_material_button_toggle__WEBPACK_IMPORTED_MODULE_10__["MatButtonToggleModule"],
            _angular_material_card__WEBPACK_IMPORTED_MODULE_11__["MatCardModule"],
            _angular_material_checkbox__WEBPACK_IMPORTED_MODULE_12__["MatCheckboxModule"],
            _angular_material_chips__WEBPACK_IMPORTED_MODULE_13__["MatChipsModule"],
            _angular_material_stepper__WEBPACK_IMPORTED_MODULE_14__["MatStepperModule"],
            _angular_material_datepicker__WEBPACK_IMPORTED_MODULE_15__["MatDatepickerModule"],
            _angular_material_dialog__WEBPACK_IMPORTED_MODULE_16__["MatDialogModule"],
            _angular_material_divider__WEBPACK_IMPORTED_MODULE_17__["MatDividerModule"],
            _angular_material_expansion__WEBPACK_IMPORTED_MODULE_18__["MatExpansionModule"],
            _angular_material_grid_list__WEBPACK_IMPORTED_MODULE_19__["MatGridListModule"],
            _angular_material_icon__WEBPACK_IMPORTED_MODULE_20__["MatIconModule"],
            _angular_material_input__WEBPACK_IMPORTED_MODULE_21__["MatInputModule"],
            _angular_material_list__WEBPACK_IMPORTED_MODULE_22__["MatListModule"],
            _angular_material_menu__WEBPACK_IMPORTED_MODULE_23__["MatMenuModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_24__["MatNativeDateModule"],
            _angular_material_paginator__WEBPACK_IMPORTED_MODULE_25__["MatPaginatorModule"],
            _angular_material_progress_bar__WEBPACK_IMPORTED_MODULE_26__["MatProgressBarModule"],
            _angular_material_progress_spinner__WEBPACK_IMPORTED_MODULE_27__["MatProgressSpinnerModule"],
            _angular_material_radio__WEBPACK_IMPORTED_MODULE_28__["MatRadioModule"],
            _angular_material_core__WEBPACK_IMPORTED_MODULE_24__["MatRippleModule"],
            _angular_material_select__WEBPACK_IMPORTED_MODULE_29__["MatSelectModule"],
            _angular_material_sidenav__WEBPACK_IMPORTED_MODULE_30__["MatSidenavModule"],
            _angular_material_slider__WEBPACK_IMPORTED_MODULE_31__["MatSliderModule"],
            _angular_material_slide_toggle__WEBPACK_IMPORTED_MODULE_32__["MatSlideToggleModule"],
            _angular_material_snack_bar__WEBPACK_IMPORTED_MODULE_33__["MatSnackBarModule"],
            _angular_material_sort__WEBPACK_IMPORTED_MODULE_34__["MatSortModule"],
            _angular_material_table__WEBPACK_IMPORTED_MODULE_35__["MatTableModule"],
            _angular_material_tabs__WEBPACK_IMPORTED_MODULE_36__["MatTabsModule"],
            _angular_material_toolbar__WEBPACK_IMPORTED_MODULE_37__["MatToolbarModule"],
            _angular_material_tooltip__WEBPACK_IMPORTED_MODULE_38__["MatTooltipModule"],
            _angular_material_tree__WEBPACK_IMPORTED_MODULE_39__["MatTreeModule"],
            src_app_system_system_routing_module__WEBPACK_IMPORTED_MODULE_53__["SystemRoutingModule"]
        ]
    })
], SystemModule);



/***/ }),

/***/ "./src/app/system/timer-plan-list/timer-plan-list.component.css":
/*!**********************************************************************!*\
  !*** ./src/app/system/timer-plan-list/timer-plan-list.component.css ***!
  \**********************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 160px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3RpbWVyLXBsYW4tbGlzdC90aW1lci1wbGFuLWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9zeXN0ZW0vdGltZXItcGxhbi1saXN0L3RpbWVyLXBsYW4tbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTYwcHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/system/timer-plan-list/timer-plan-list.component.ts":
/*!*********************************************************************!*\
  !*** ./src/app/system/timer-plan-list/timer-plan-list.component.ts ***!
  \*********************************************************************/
/*! exports provided: TimerPlanListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TimerPlanListComponent", function() { return TimerPlanListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _timer_plan_service__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../timer-plan.service */ "./src/app/system/timer-plan.service.ts");





let TimerPlanListComponent = class TimerPlanListComponent {
    constructor(TimerPlanService, snackBar, location) {
        this.TimerPlanService = TimerPlanService;
        this.snackBar = snackBar;
        this.location = location;
        this.TimerPlans = [];
        this.tableColumns = ['TimerPlanID', 'Name', 'Active'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.TimerPlanService.getTimerPlans().subscribe((TimerPlansMsg) => {
            this.TimerPlans = TimerPlansMsg.TimerPlans;
            console.log(this.TimerPlans);
            this.snackBar.open(TimerPlansMsg.Count + ' Timer Plan' + (this.TimerPlans.length > 1 ? 's' : '') + ' loaded. ', '', { duration: 3000 });
            return this.TimerPlans;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Timer Plans found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
TimerPlanListComponent.ctorParameters = () => [
    { type: _timer_plan_service__WEBPACK_IMPORTED_MODULE_4__["TimerPlanService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_3__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] }
];
TimerPlanListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-timer-plan-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./timer-plan-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/timer-plan-list/timer-plan-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./timer-plan-list.component.css */ "./src/app/system/timer-plan-list/timer-plan-list.component.css")).default]
    })
], TimerPlanListComponent);



/***/ }),

/***/ "./src/app/system/timer-plan.service.ts":
/*!**********************************************!*\
  !*** ./src/app/system/timer-plan.service.ts ***!
  \**********************************************/
/*! exports provided: TimerPlanService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TimerPlanService", function() { return TimerPlanService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let TimerPlanService = class TimerPlanService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getTimerPlans() {
        return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'TimerPlans');
    }
    getTimerPlan(TimerPlanId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'TimerPlans'}/${TimerPlanId}`);
    }
};
TimerPlanService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
TimerPlanService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], TimerPlanService);



/***/ }),

/***/ "./src/app/system/timer-plan/timer-plan.component.css":
/*!************************************************************!*\
  !*** ./src/app/system/timer-plan/timer-plan.component.css ***!
  \************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS90aW1lci1wbGFuL3RpbWVyLXBsYW4uY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/system/timer-plan/timer-plan.component.ts":
/*!***********************************************************!*\
  !*** ./src/app/system/timer-plan/timer-plan.component.ts ***!
  \***********************************************************/
/*! exports provided: TimerPlanComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "TimerPlanComponent", function() { return TimerPlanComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let TimerPlanComponent = class TimerPlanComponent {
    constructor() { }
    ngOnInit() {
    }
};
TimerPlanComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-timer-plan',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./timer-plan.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/timer-plan/timer-plan.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./timer-plan.component.css */ "./src/app/system/timer-plan/timer-plan.component.css")).default]
    })
], TimerPlanComponent);



/***/ }),

/***/ "./src/app/system/unit-list/unit-list.component.css":
/*!**********************************************************!*\
  !*** ./src/app/system/unit-list/unit-list.component.css ***!
  \**********************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = (":host {\r\n  width: 100%;\r\n  margin: 12px auto auto auto;\r\n}\r\n\r\n.mat-row:hover {\r\n  background-color: lightgray;\r\n  border-radius: 4px;\r\n}\r\n\r\n.mat-card-subtitle {\r\n  display: block;\r\n  margin-bottom: 0px;\r\n  min-width: 120px;\r\n}\r\n\r\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbInNyYy9hcHAvc3lzdGVtL3VuaXQtbGlzdC91bml0LWxpc3QuY29tcG9uZW50LmNzcyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFBQTtFQUNFLFdBQVc7RUFDWCwyQkFBMkI7QUFDN0I7O0FBRUE7RUFDRSwyQkFBMkI7RUFDM0Isa0JBQWtCO0FBQ3BCOztBQUVBO0VBQ0UsY0FBYztFQUNkLGtCQUFrQjtFQUNsQixnQkFBZ0I7QUFDbEIiLCJmaWxlIjoic3JjL2FwcC9zeXN0ZW0vdW5pdC1saXN0L3VuaXQtbGlzdC5jb21wb25lbnQuY3NzIiwic291cmNlc0NvbnRlbnQiOlsiOmhvc3Qge1xyXG4gIHdpZHRoOiAxMDAlO1xyXG4gIG1hcmdpbjogMTJweCBhdXRvIGF1dG8gYXV0bztcclxufVxyXG5cclxuLm1hdC1yb3c6aG92ZXIge1xyXG4gIGJhY2tncm91bmQtY29sb3I6IGxpZ2h0Z3JheTtcclxuICBib3JkZXItcmFkaXVzOiA0cHg7XHJcbn1cclxuXHJcbi5tYXQtY2FyZC1zdWJ0aXRsZSB7XHJcbiAgZGlzcGxheTogYmxvY2s7XHJcbiAgbWFyZ2luLWJvdHRvbTogMHB4O1xyXG4gIG1pbi13aWR0aDogMTIwcHg7XHJcbn1cclxuIl19 */");

/***/ }),

/***/ "./src/app/system/unit-list/unit-list.component.ts":
/*!*********************************************************!*\
  !*** ./src/app/system/unit-list/unit-list.component.ts ***!
  \*********************************************************/
/*! exports provided: UnitListComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UnitListComponent", function() { return UnitListComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common */ "./node_modules/@angular/common/fesm2015/common.js");
/* harmony import */ var _angular_material__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/material */ "./node_modules/@angular/material/esm2015/material.js");
/* harmony import */ var _unit_service__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ../unit.service */ "./src/app/system/unit.service.ts");





let UnitListComponent = class UnitListComponent {
    constructor(UnitService, snackBar, location) {
        this.UnitService = UnitService;
        this.snackBar = snackBar;
        this.location = location;
        this.Units = [];
        this.tableColumns = ['UnitID', 'Name', 'Minimum', 'Maximum', 'IconList', 'TextLabels'];
        this.selectedRow = 0;
        this.selectedRowNumber = 0;
    }
    goBack() {
        this.location.back();
        console.log('goBack()...');
    }
    ngOnInit() {
        this.UnitService.getUnits().subscribe((UnitsMsg) => {
            this.Units = UnitsMsg.Units;
            console.log(this.Units);
            this.snackBar.open(UnitsMsg.Count + ' Unit' + (this.Units.length > 1 ? 's' : '') + ' loaded. ', '', { duration: 3000 });
            return this.Units;
        }, (error) => {
            if (error.status == 404) {
                this.snackBar.open('No Units found. ', '', { duration: 5000 });
            }
            else {
                this.snackBar.open(error.statusText, '', { duration: 10000 });
            }
        });
    }
};
UnitListComponent.ctorParameters = () => [
    { type: _unit_service__WEBPACK_IMPORTED_MODULE_4__["UnitService"] },
    { type: _angular_material__WEBPACK_IMPORTED_MODULE_3__["MatSnackBar"] },
    { type: _angular_common__WEBPACK_IMPORTED_MODULE_2__["Location"] }
];
UnitListComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-unit-list',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./unit-list.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/unit-list/unit-list.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./unit-list.component.css */ "./src/app/system/unit-list/unit-list.component.css")).default]
    })
], UnitListComponent);



/***/ }),

/***/ "./src/app/system/unit.service.ts":
/*!****************************************!*\
  !*** ./src/app/system/unit.service.ts ***!
  \****************************************/
/*! exports provided: UnitService */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UnitService", function() { return UnitService; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_common_http__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/common/http */ "./node_modules/@angular/common/fesm2015/http.js");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ../../environments/environment */ "./src/environments/environment.ts");




let UnitService = class UnitService {
    constructor(httpClient) {
        this.httpClient = httpClient;
    }
    getUnits() {
        return this.httpClient.get(_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Units');
    }
    getUnit(UnitId) {
        return this.httpClient.get(`${_environments_environment__WEBPACK_IMPORTED_MODULE_3__["environment"].API_URL + 'Units'}/${UnitId}`);
    }
};
UnitService.ctorParameters = () => [
    { type: _angular_common_http__WEBPACK_IMPORTED_MODULE_2__["HttpClient"] }
];
UnitService = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Injectable"])({
        providedIn: 'root'
    })
], UnitService);



/***/ }),

/***/ "./src/app/system/unit/unit.component.css":
/*!************************************************!*\
  !*** ./src/app/system/unit/unit.component.css ***!
  \************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS91bml0L3VuaXQuY29tcG9uZW50LmNzcyJ9 */");

/***/ }),

/***/ "./src/app/system/unit/unit.component.ts":
/*!***********************************************!*\
  !*** ./src/app/system/unit/unit.component.ts ***!
  \***********************************************/
/*! exports provided: UnitComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "UnitComponent", function() { return UnitComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let UnitComponent = class UnitComponent {
    constructor() { }
    ngOnInit() {
    }
};
UnitComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-unit',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./unit.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/unit/unit.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./unit.component.css */ "./src/app/system/unit/unit.component.css")).default]
    })
], UnitComponent);



/***/ }),

/***/ "./src/app/system/value-timer/value-timer.component.css":
/*!**************************************************************!*\
  !*** ./src/app/system/value-timer/value-timer.component.css ***!
  \**************************************************************/
/*! exports provided: default */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony default export */ __webpack_exports__["default"] = ("\n/*# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJzb3VyY2VzIjpbXSwibmFtZXMiOltdLCJtYXBwaW5ncyI6IiIsImZpbGUiOiJzcmMvYXBwL3N5c3RlbS92YWx1ZS10aW1lci92YWx1ZS10aW1lci5jb21wb25lbnQuY3NzIn0= */");

/***/ }),

/***/ "./src/app/system/value-timer/value-timer.component.ts":
/*!*************************************************************!*\
  !*** ./src/app/system/value-timer/value-timer.component.ts ***!
  \*************************************************************/
/*! exports provided: ValueTimerComponent */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "ValueTimerComponent", function() { return ValueTimerComponent; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");


let ValueTimerComponent = class ValueTimerComponent {
    constructor() { }
    ngOnInit() {
    }
};
ValueTimerComponent = tslib__WEBPACK_IMPORTED_MODULE_0__["__decorate"]([
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_1__["Component"])({
        selector: 'dmz-value-timer',
        template: tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! raw-loader!./value-timer.component.html */ "./node_modules/raw-loader/dist/cjs.js!./src/app/system/value-timer/value-timer.component.html")).default,
        styles: [tslib__WEBPACK_IMPORTED_MODULE_0__["__importDefault"](__webpack_require__(/*! ./value-timer.component.css */ "./src/app/system/value-timer/value-timer.component.css")).default]
    })
], ValueTimerComponent);



/***/ }),

/***/ "./src/environments/environment.ts":
/*!*****************************************!*\
  !*** ./src/environments/environment.ts ***!
  \*****************************************/
/*! exports provided: environment */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export (binding) */ __webpack_require__.d(__webpack_exports__, "environment", function() { return environment; });
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
// This file can be replaced during build by using the `fileReplacements` array.
// `ng build --prod` replaces `environment.ts` with `environment.prod.ts`.
// The list of file replacements can be found in `angular.json`.

const environment = {
    production: false,
    API_URL: "HTTP://192.168.0.19:8080/API/"
};
/*
 * For easier debugging in development mode, you can import the following file
 * to ignore zone related error stack frames such as `zone.run`, `zoneDelegate.invokeTask`.
 *
 * This import should be commented out in production mode because it will have a negative impact
 * on performance if an error is thrown.
 */
// import 'zone.js/dist/zone-error';  // Included with Angular CLI.


/***/ }),

/***/ "./src/main.ts":
/*!*********************!*\
  !*** ./src/main.ts ***!
  \*********************/
/*! no exports provided */
/***/ (function(module, __webpack_exports__, __webpack_require__) {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony import */ var tslib__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! tslib */ "./node_modules/tslib/tslib.es6.js");
/* harmony import */ var hammerjs__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! hammerjs */ "./node_modules/hammerjs/hammer.js");
/* harmony import */ var hammerjs__WEBPACK_IMPORTED_MODULE_1___default = /*#__PURE__*/__webpack_require__.n(hammerjs__WEBPACK_IMPORTED_MODULE_1__);
/* harmony import */ var _angular_core__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! @angular/core */ "./node_modules/@angular/core/fesm2015/core.js");
/* harmony import */ var _angular_platform_browser_dynamic__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! @angular/platform-browser-dynamic */ "./node_modules/@angular/platform-browser-dynamic/fesm2015/platform-browser-dynamic.js");
/* harmony import */ var _app_app_module__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! ./app/app.module */ "./src/app/app.module.ts");
/* harmony import */ var _environments_environment__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! ./environments/environment */ "./src/environments/environment.ts");






if (_environments_environment__WEBPACK_IMPORTED_MODULE_5__["environment"].production) {
    Object(_angular_core__WEBPACK_IMPORTED_MODULE_2__["enableProdMode"])();
}
Object(_angular_platform_browser_dynamic__WEBPACK_IMPORTED_MODULE_3__["platformBrowserDynamic"])().bootstrapModule(_app_app_module__WEBPACK_IMPORTED_MODULE_4__["AppModule"])
    .catch(err => console.error(err));


/***/ }),

/***/ 0:
/*!***************************!*\
  !*** multi ./src/main.ts ***!
  \***************************/
/*! no static exports found */
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__(/*! D:\domoticz\web\angular8\src\main.ts */"./src/main.ts");


/***/ })

},[[0,"runtime","vendor"]]]);
//# sourceMappingURL=main-es2015.js.map