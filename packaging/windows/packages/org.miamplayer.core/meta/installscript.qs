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
	installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
	var programFiles = installer.environmentVariable("PROGRAMW6432");
	if (programFiles !== "") {
		installer.setValue("TargetDir", programFiles + "/MiamPlayer");
	}
}

// Utility function
var Dir = new function () {
    this.toNativeSparator = function (path) {
        if (installer.value("os") == "win")
            return path.replace(/\//g, '\\');
        return path;
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

            widget.windowTitle = "Installation Folder";
            widget.targetDirectory.text = Dir.toNativeSparator(installer.value("TargetDir"));
        }
    }

	// don't show when updating / de-installing
	if (installer.isInstaller()) {
		installer.addWizardPageItem(component, "CopyBitcoinAddressForm", QInstaller.InstallationFinished);
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
}

Component.prototype.chooseTarget = function () {
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget != null) {
        var newTarget = QFileDialog.getExistingDirectory("Choose your target directory.", widget.targetDirectory.text);
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
			if (installer.fileExists(text + "/components.xml")) {
				var warning = "<font color='red'>" + qsTr("A previous installation exists in this folder. If you wish to continue, everything will be overwritten.") + "</font>";
				widget.labelOverwrite.text = warning;
			} else {
				widget.labelOverwrite.text = "";
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
		
		if (installer.value("os") == "win") { 
			try {
				component.addOperation("CreateShortcut", "@TargetDir@\\MiamPlayer.exe", "@StartMenuDir@\\MiamPlayer.lnk");
				component.addElevatedOperation("Execute", "@TargetDir@\\vcredist\\vc2012_redist_x64.exe", "/norestart", "/q");
				component.addElevatedOperation("Execute", "@TargetDir@\\vcredist\\vc2013_redist_x64.exe", "/norestart", "/q");
				component.addElevatedOperation("Execute", "@TargetDir@\\vcredist\\vc2013_redist_x64.exe", "/norestart", "/q");
				component.addElevatedOperation("RegisterFileType", "@TargetDir@\\vcredist\\vc2013_redist_x64.exe", "/norestart", "/q");
				component.addOperation("RegisterFileType",
                               "mp3",
                               '@TargetDir@\\MiamPlayer.exe -f "%1"',
                               "Miam-Player media file (*.mp3)",
                               "audio/mpeg",
                               "@TargetDir@\\MiamPlayer.exe," + 0,
                               "ProgId=MiamPlayer.mp3");
				
				// Always clear registry after install (should be improved)
				// component.addElevatedOperation("Execute", 'REG DELETE "HKCU\\Software\\MmeMiamMiam" /F');
			} catch (e) {
				// Do nothing if key doesn't exist
			}
		}
	} catch (e) {
		print(e);
	}
}
