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
}

// called as soon as the component was loaded
Component.prototype.installerLoaded = function()
{
	// don't show when updating / de-installing
	if (installer.isInstaller()) {

		if (installer.addWizardPage(component, "TargetWidget", QInstaller.TargetDirectory)) {
			var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
            if (widget !== null) {
				widget.targetDirectory.textChanged.connect(this, Component.prototype.targetChanged);
				widget.targetChooser.clicked.connect(this, Component.prototype.chooseTarget);

				widget.windowTitle = qsTr("Installation Folder");
				widget.targetDirectory.text = installer.value("TargetDir");
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
	
    if (installer.value("os") === "x11") {
        installer.setValue("RunProgram", "@TargetDir@/miam-player");
    }
}

Component.prototype.chooseTarget = function () {
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget !== null) {
        var newTarget = QFileDialog.getExistingDirectory(qsTr("Choose your target directory."), widget.targetDirectory.text);
        if (newTarget !== "") {
            widget.targetDirectory.text = newTarget;
		}
    }
}

Component.prototype.targetChanged = function (text) {
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget !== null) {
        if (text !== "") {
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
        if (installer.value("os") === "x11") {
            try {
                component.addOperation("Execute", "@TargetDir@/generate_symbolic_links.sh", "@TargetDir@");
                component.addElevatedOperation("Execute", "touch", "/etc/ld.so.conf.d/miam-player.conf");
                component.addElevatedOperation("AppendFile", "/etc/ld.so.conf.d/miam-player.conf", "@TargetDir@");
                component.addElevatedOperation("Execute", "ldconfig");
            } catch (e) {
                print(e);
            }
        }
	} catch (e) {
		print(e);
	}
}
