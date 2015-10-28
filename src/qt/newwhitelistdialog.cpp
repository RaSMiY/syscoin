#include "newwhitelistdialog.h"
#include "ui_newwhitelistdialog.h"
#include "offertablemodel.h"
#include "offerwhitelisttablemodel.h"
#include "guiutil.h"
#include "walletmodel.h"
#include "bitcoingui.h"
#include "ui_interface.h"
#include "bitcoinrpc.h"
#include "script.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QStringList>
using namespace std;
using namespace json_spirit;
extern int nBestHeight;
extern const CRPCTable tableRPC;
int64 GetOfferNetworkFee(opcodetype seed, unsigned int nHeight);
NewWhitelistDialog::NewWhitelistDialog(const QString &offerGUID, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewWhitelistDialog), model(0)
{
    ui->setupUi(this);
	ui->offerGUIDLabel->setText(offerGUID);
	ui->discountDisclaimer->setText(tr("<font color='red'>This is a percentage of price for your offer you want to allow your reseller to purchase your offer for. Typically given to wholesalers or for special arrangements with a reseller.</font>"));
	ui->certEdit->setEditText(tr("Choose or enter GUID of a Certificate you own"));
	loadCerts();
}

NewWhitelistDialog::~NewWhitelistDialog()
{
    delete ui;
}


void NewWhitelistDialog::setModel(WalletModel *walletModel, OfferWhitelistTableModel *model)
{
    this->model = model;
	this->walletModel = walletModel;
}
void NewWhitelistDialog::loadCerts()
{
	string strMethod = string("certlist");
    Array params; 
	Value result ;
	string name_str;
	string title_str;
	int expired = 0;
	
	try {
		result = tableRPC.execute(strMethod, params);

		if (result.type() == array_type)
		{
			name_str = "";
			expired = 0;


	
			Array arr = result.get_array();
			BOOST_FOREACH(Value& input, arr)
			{
				if (input.type() != obj_type)
					continue;
				Object& o = input.get_obj();
				name_str = "";
				title_str = "";
				expired = 0;


		
				const Value& name_value = find_value(o, "cert");
				if (name_value.type() == str_type)
					name_str = name_value.get_str();
				const Value& title_value = find_value(o, "title");
				if (title_value.type() == str_type)
					title_str = title_value.get_str();					
				const Value& expired_value = find_value(o, "expired");
				if (expired_value.type() == int_type)
					expired = expired_value.get_int();
				
				if(expired == 0)
				{
					QString name = QString::fromStdString(name_str);
					QString title = QString::fromStdString(title_str);
					QString certText = name + " - " + title;
					ui->certEdit->addItem(certText,name);
				}
				
			}
		}
	}
	catch (Object& objError)
	{
		string strError = find_value(objError, "message").get_str();
		QMessageBox::critical(this, windowTitle(),
			tr("Could not refresh cert list: %1").arg(QString::fromStdString(strError)),
				QMessageBox::Ok, QMessageBox::Ok);
	}
	catch(std::exception& e)
	{
		QMessageBox::critical(this, windowTitle(),
			tr("There was an exception trying to refresh the cert list: ") + QString::fromStdString(e.what()),
				QMessageBox::Ok, QMessageBox::Ok);
	}         
 
}
bool NewWhitelistDialog::saveCurrentRow()
{

    if(!model || !walletModel) return false;
    WalletModel::UnlockContext ctx(walletModel->requestUnlock());
    if(!ctx.isValid())
    {
		model->editStatus = OfferWhitelistTableModel::WALLET_UNLOCK_FAILURE;
        return false;
    }
	Array params;
	string strMethod;
	double updateFee;
	string updateFeeStr;
	QMessageBox::StandardButton retval;

    
	updateFee = (double)GetOfferNetworkFee(OP_OFFER_UPDATE, nBestHeight)/(double)COIN;
	updateFeeStr = strprintf("%.2f", updateFee);
    retval = QMessageBox::question(this, tr("Confirm whitelist entry"),
             tr("Warning: Updating an offer whitelist will cost ") + QString::fromStdString(updateFeeStr) + " SYS<br><br>" + tr("Do you want to continue?"),
             QMessageBox::Yes|QMessageBox::Cancel,
             QMessageBox::Cancel);
	if(retval != QMessageBox::Yes)
	{
		return false;
	}
	strMethod = string("offeraddwhitelist");
	params.push_back(ui->offerGUIDLabel->text().toStdString());
	if(ui->certEdit->currentIndex() >= 0)
		params.push_back(ui->certEdit->itemData(ui->certEdit->currentIndex()).toString().toStdString());
	params.push_back(ui->discountEdit->text().toStdString());

	try {
        Value result = tableRPC.execute(strMethod, params);
		if(ui->certEdit->currentIndex() >= 0)
			entry = ui->certEdit->itemData(ui->certEdit->currentIndex()).toString();

		QMessageBox::information(this, windowTitle(),
        tr("New whitelist entry added successfully!"),
			QMessageBox::Ok, QMessageBox::Ok);
			
		
	}
	catch (Object& objError)
	{
		string strError = find_value(objError, "message").get_str();
		QMessageBox::critical(this, windowTitle(),
		tr("Error creating new whitelist entry: \"%1\"").arg(QString::fromStdString(strError)),
			QMessageBox::Ok, QMessageBox::Ok);
	}
	catch(std::exception& e)
	{
		QMessageBox::critical(this, windowTitle(),
			tr("General exception creating new whitelist entry: \"%1\"").arg(QString::fromStdString(e.what())),
			QMessageBox::Ok, QMessageBox::Ok);
	}							

    return !entry.isEmpty();
}

void NewWhitelistDialog::accept()
{
    if(!model) return;

    if(!saveCurrentRow())
    {
        switch(model->getEditStatus())
        {
        case OfferWhitelistTableModel::OK:
            // Failed with unknown reason. Just reject.
            break;
        case OfferWhitelistTableModel::NO_CHANGES:
            // No changes were made during edit operation. Just reject.
            break;
        case OfferWhitelistTableModel::INVALID_ENTRY:
            QMessageBox::warning(this, windowTitle(),
                tr("The entered entry \"%1\" is not a valid whitelist entry.").arg(ui->certEdit->currentText()),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case OfferWhitelistTableModel::DUPLICATE_ENTRY:
            QMessageBox::warning(this, windowTitle(),
                tr("The entered entry \"%1\" is already taken.").arg(ui->certEdit->currentText()),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case OfferWhitelistTableModel::WALLET_UNLOCK_FAILURE:
            QMessageBox::critical(this, windowTitle(),
                tr("Could not unlock wallet."),
                QMessageBox::Ok, QMessageBox::Ok);
            break;

        }
        return;
    }
    QDialog::accept();
}

QString NewWhitelistDialog::getEntry() const
{
    return entry;
}

