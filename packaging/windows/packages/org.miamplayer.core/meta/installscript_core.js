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
	if (installer.isInstaller()) {
		installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
	} else {
		installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
	}
	var programFiles = installer.environmentVariable("PROGRAMW6432");
	if (programFiles !== "") {
		installer.setValue("TargetDir", programFiles + "/MiamPlayer");
	}
}

// Utility function
var Dir = new function () {
    this.toNativeSparator = function (path) {
        if (systemInfo.productType === "windows")
            return path.replace(/\//g, '\\');
        return path;
    }
}; 

// called as soon as the component was loaded
Component.prototype.installerLoaded = function()
{
	// don't show when updating / de-installing
	if (installer.isInstaller()) {

		if (installer.addWizardPage(component, "TargetWidget", QInstaller.TargetDirectory)) {
			var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
			if (widget != null) {
				widget.targetDirectory.textChanged.connect(this, Component.prototype.targetChanged);
				widget.targetChooser.clicked.connect(this, Component.prototype.chooseTarget);

				widget.windowTitle = qsTr("Installation Folder");
				widget.targetDirectory.text = Dir.toNativeSparator(installer.value("TargetDir"));
			}
		}
		installer.addWizardPageItem(component, "CopyBitcoinAddressForm", QInstaller.InstallationFinished);

	} else {
	
		if (installer.addWizardPage(component, "ReadyForInstallationWidget", QInstaller.ReadyForInstallation)) {
			var widget = gui.pageWidgetByObjectName("DynamicReadyForInstallationWidget");
		}
	}
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
	
	if (systemInfo.productType === "windows") {
        installer.setValue("RunProgram", "@TargetDir@\\MiamPlayer.exe");
    } else {
        installer.setValue("RunProgram", "@TargetDir@/MiamPlayer");
    }
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
			installer.setValue("TargetDir", text);
			if (!installer.fileExists(text + "/components.xml")) {
				widget.labelOverwrite.visible = false;
				widget.labelUserShouldRemove.visible = false;
				widget.clearCacheCheckBox.visible = false;
				widget.clearRegistryCheckBox.visible = false;
				installer.setValue("softWasInstalledBefore", "0");
			} else {
				installer.setValue("softWasInstalledBefore", "1");
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
		if (systemInfo.productType === "windows") { 
			try {
				// only install c runtime if it is needed, no minor version check of the c runtime till we need it
				// return value 3010 means it need a reboot, but in most cases it is not needed for run Qt application
				// return value 5100 means there's a newer version of the runtime already installed
				if (installer.value("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\11.0\\VC\\Runtimes\\x64\\Installed") != 1) {
					component.addElevatedOperation("Execute", "{0,1638,3010,5100}", "@TargetDir@\\vcredist\\vc2012_redist_x64.exe", "/norestart", "/q");
				}
				if (installer.value("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\12.0\\VC\\Runtimes\\x64\\Installed") != 1) {
					component.addElevatedOperation("Execute", "{0,1638,3010,5100}", "@TargetDir@\\vcredist\\vc2013_redist_x64.exe", "/norestart", "/q");
				}
				var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
				if (widget.associateCommonFiletypesCheckBox.checked) {
					var index;
					var extensions = ["ape", "asf", "flac", "m4a", "mpc", "mp3", "oga", "ogg", "opus"];
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
				// Remove cache (folder including subfolders, sqlite database, etc)
				var home = installer.environmentVariable("USERPROFILE");
				if (widget.clearCacheCheckBox.checked && installer.value("softWasInstalledBefore") == "1") {
					/// FIXME
					//component.addElevatedOperation("Rmdir", home + "\\AppData\\Local\\MmeMiamMiam");
					component.addElevatedOperation("Delete", home + "\\AppData\\Local\\MmeMiamMiam\\MiamPlayer\\mp.db");
				}
				// Remove ini file
				if (widget.clearRegistryCheckBox.checked && installer.value("softWasInstalledBefore") == "1") {
					// Return codes are "0" == OK and "1" == KO even if a problem has occured. "1" can happen when one has manually deleted settings
					/// FIXME
					//component.addElevatedOperation("Rmdir", home + "\\AppData\\Roaming\\MmeMiamMiam");
					component.addElevatedOperation("Delete", home + "\\AppData\\Roaming\\MmeMiamMiam\\MiamPlayer.ini");
				}
			} catch (e) {
				// Do nothing if key doesn't exist
			}
		}
	} catch (e) {
		print(e);
	}
}
