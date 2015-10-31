/**************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
**************************************************************************/

// Constructor
function Component()
{
	component.loaded.connect(this, Component.prototype.installerLoaded);
	var programFiles = installer.environmentVariable("PROGRAMW6432");
	if (programFiles !== "") {
		installer.setValue("TargetDir", programFiles + "/MiamPlayer");
	}
}

// Utility function
var Dir = new function () {
    this.toNativeSparator = function (path) {
		return path.replace(/\//g, '\\');
    }
}; 

// called as soon as the component was loaded
Component.prototype.installerLoaded = function()
{
	if (installer.addWizardPage(component, "TargetWidget", QInstaller.TargetDirectory)) {
		var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
		if (widget != null) {
			widget.targetDirectory.textChanged.connect(this, Component.prototype.targetChanged);
			widget.targetChooser.clicked.connect(this, Component.prototype.chooseTarget);

			widget.windowTitle = qsTr("Installation Folder");
			widget.targetDirectory.text = Dir.toNativeSparator(installer.value("TargetDir"));
			
			// Check if there are previous cache and settings files
			var home = installer.environmentVariable("USERPROFILE");
			if (installer.fileExists(home + "\\AppData\\Local\\MmeMiamMiam\\MiamPlayer\\mp.db")) {
				installer.setValue("cacheExists", "1");
				widget.clearCacheCheckBox.visible = true;
			} else {
				installer.setValue("cacheExists", "0");
				widget.clearCacheCheckBox.visible = false;				
			}
			if (installer.fileExists(home + "\\AppData\\Roaming\\MmeMiamMiam\\MiamPlayer.ini")) {
				installer.setValue("settingsExists", "1");
				widget.clearSettingsCheckBox.visible = true;
			} else {
				installer.setValue("settingsExists", "0");
				widget.clearSettingsCheckBox.visible = false;
			}
			
			// Don't show label if nothing was detected
			if (installer.value("cacheExists") === "1" || installer.value("settingsExists") === "1") {
				widget.labelWarningPrevFilesImg.visible = true;
				widget.labelWarningPrevFilesText.visible = true;
			} else {
				widget.labelWarningPrevFilesImg.visible = false;
				widget.labelWarningPrevFilesText.visible = false;
			}
		}
	}
	installer.addWizardPageItem(component, "CopyBitcoinAddressForm", QInstaller.InstallationFinished);
}

Component.prototype.isDefault = function()
{
	// select the component by default
	return true;
}

// called after everything is set up, but before any file is written
Component.prototype.beginInstallation = function()
{
    // call default implementation which is necessary for most hooks in beginInstallation case it makes nothing
    component.beginInstallation();
	installer.setValue("RunProgram", "@TargetDir@\\MiamPlayer.exe");
}

Component.prototype.chooseTarget = function ()
{
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget != null) {
        var newTarget = QFileDialog.getExistingDirectory(qsTr("Choose your target directory."), widget.targetDirectory.text);
        if (newTarget != "") {
            widget.targetDirectory.text = Dir.toNativeSparator(newTarget);
		}
    }
}

Component.prototype.targetChanged = function (text) {
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget != null) {
        if (text != "") {
			widget.complete = true;
			installer.setValue("TargetDir", Dir.toNativeSparator(text));
			if (installer.fileExists(text + "/components.xml")) {
				installer.setValue("softWasInstalledBefore", "1");
			} else {
				widget.labelOverwrite.visible = false;
				installer.setValue("softWasInstalledBefore", "0");
			}
			return;
        }
        widget.complete = false;
    }
}

Component.prototype.createOperations = function()
{
	try {
		// call the base create operations function
		component.createOperations();
		var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
		if (widget.associateCommonFiletypesCheckBox.checked) {
			var index;
			var extensions = ["ape", "asf", "flac", "m4a", "mp4", "mpc", "mp3", "oga", "ogg", "opus"];
			for (index = 0; index < extensions.length; ++index) {
				var ext = extensions[index];
				component.addOperation("RegisterFileType", ext,
									   '@TargetDir@\\MiamPlayer.exe -f "%1"',
									   "Miam-Player media file (*." + ext + ")",
									   "audio/mpeg",
									   "@TargetDir@\\MiamPlayer.exe," + 0,
									   "ProgId=MiamPlayer." + ext);
			}
		}
		// Remove cache (folder including subfolders, sqlite database, etc) /// FIXME: removing folders
		var home = installer.environmentVariable("USERPROFILE");
		if (widget.clearCacheCheckBox.checked || installer.value("cacheExists") === "1") {
			component.addElevatedOperation("Delete", home + "\\AppData\\Local\\MmeMiamMiam\\MiamPlayer\\mp.db");
		}
		// Remove ini file
		if (widget.clearSettingsCheckBox.checked || installer.value("settingsExists") === "1") {
			// Return codes are "0" == OK and "1" == KO even if a problem has occured. "1" can happen when one has manually deleted settings
			component.addElevatedOperation("Delete", home + "\\AppData\\Roaming\\MmeMiamMiam\\MiamPlayer.ini");
		}
	} catch (e) {
		print(e);
	}
}
