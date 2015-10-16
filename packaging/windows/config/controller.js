function Controller()
{
	if (installer.isUninstaller()) {
		installer.setDefaultPageVisible(QInstaller.Introduction, false);
		installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
		installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
		
		//installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
		//installer.addWizardPage(component, "RemoveEverything", QInstaller.PerformInstallation);
	}
}

// Select by default "I accept" the Licence Agreement. Seriously, who doesn't?
Controller.prototype.LicenseAgreementPageCallback = function()
{
	var widget = gui.currentPageWidget();
	if (widget != null) {
		widget.AcceptLicenseRadioButton.checked = true;
	}
}

/*Controller.prototype.DynamicRemoveEverythingPageCallback = function()
{
	var widget = gui.pageWidgetByObjectName("DynamicRemoveEverything");
	if (widget != null) {
		var r = QMessageBox["question"]("q", "Installer", "uninstaller == true", QMessageBox.Ok);
	}
}*/

/*Controller.prototype.ReadyForInstallationPageCallback = function()
{
	if (installer.isUninstaller()) {
		var widget = gui.currentPageWidget();
		if (widget != null) {
			//var rr = QMessageBox["question"]("q", "Installer", "widget != null", QMessageBox.Ok);
			//widget.findChild("CancelButton").setText("Test Cancel");
			//widget.findChild("CommitButton").setText("Test Commit");
		}
	}
}*/
