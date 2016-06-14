/***************************************************************************
 *   Copyright (C) 2006-2008, 2014, 2016 by Hanna Knutsson                 *
 *   hanna_k@fmgirl.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QAction>
#include <QDateEdit>
#include <QLineEdit>
#include <QCompleter>
#include <QStandardItemModel>
#include <QStringList>
#include <QComboBox>
#include <QMessageBox>

#include <kstdguiitem.h>
#include <klocalizedstring.h>

#include "budget.h"
#include "eqonomizevalueedit.h"
#include "transactionfilterwidget.h"

#include <cmath>

extern double monthsBetweenDates(const QDate &date1, const QDate &date2);
extern double yearsBetweenDates(const QDate &date1, const QDate &date2);

TransactionFilterWidget::TransactionFilterWidget(bool extra_parameters, int transaction_type, Budget *budg, QWidget *parent) : QWidget(parent), transtype(transaction_type), budget(budg), b_extra(extra_parameters) {
	payeeEdit = NULL;
	/*int rows = 5;
	if(b_extra && (transtype == TRANSACTION_TYPE_EXPENSE || transtype == TRANSACTION_TYPE_INCOME)) rows = 6;*/
	QGridLayout *filterLayout = new QGridLayout(this);
	dateFromButton = new QCheckBox(i18n("From:"), this);
	dateFromButton->setChecked(false);
	filterLayout->addWidget(dateFromButton, 0, 0);
	dateFromEdit = new QDateEdit(QDate::currentDate(), this);
	dateFromEdit->setCalendarPopup(true);
	dateFromEdit->setEnabled(false);
	filterLayout->addWidget(dateFromEdit, 0, 1);
	filterLayout->addWidget(new QLabel(i18n("To:"), this), 0, 2);
	dateToEdit = new QDateEdit(QDate::currentDate(), this);
	dateToEdit->setCalendarPopup(true);
	filterLayout->addWidget(dateToEdit, 0, 3);
	QDate curdate = QDate::currentDate();
	from_date.setDate(curdate.year(), curdate.month(), 1);
	dateFromEdit->setDate(from_date);
	to_date = curdate;
	dateToEdit->setDate(to_date);
	if(transtype == TRANSACTION_TYPE_TRANSFER) {
		filterLayout->addWidget(new QLabel(i18n("From:"), this), 2, 0);
		fromCombo = new QComboBox(this);
		fromCombo->setEditable(false);
		filterLayout->addWidget(fromCombo, 2, 1);
		filterLayout->addWidget(new QLabel(i18n("To:"), this), 2, 2);
		toCombo = new QComboBox(this);
		toCombo->setEditable(false);
		filterLayout->addWidget(toCombo, 2, 3);
		minButton = new QCheckBox(i18n("Min amount:"), this);
		maxButton = new QCheckBox(i18n("Max amount:"), this);
	} else if(transtype == TRANSACTION_TYPE_INCOME) {
		filterLayout->addWidget(new QLabel(i18n("Category:"), this), 2, 0);
		fromCombo = new QComboBox(this);
		fromCombo->setEditable(false);
		filterLayout->addWidget(fromCombo, 2, 1);
		filterLayout->addWidget(new QLabel(i18n("To account:"), this), 2, 2);
		toCombo = new QComboBox(this);
		toCombo->setEditable(false);
		filterLayout->addWidget(toCombo, 2, 3);
		minButton = new QCheckBox(i18n("Min income:"), this);
		maxButton = new QCheckBox(i18n("Max income:"), this);
	} else {
		filterLayout->addWidget(new QLabel(i18n("Category:"), this), 2, 0);
		toCombo = new QComboBox(this);
		toCombo->setEditable(false);
		filterLayout->addWidget(toCombo, 2, 1);
		filterLayout->addWidget(new QLabel(i18n("From account:"), this), 2, 2);
		fromCombo = new QComboBox(this);
		fromCombo->setEditable(false);
		filterLayout->addWidget(fromCombo, 2, 3);
		minButton = new QCheckBox(i18n("Min cost:"), this);
		maxButton = new QCheckBox(i18n("Max cost:"), this);
	}
	filterLayout->addWidget(minButton, 1, 0);
	minEdit = new EqonomizeValueEdit(false, this);
	minEdit->setEnabled(false);
	filterLayout->addWidget(minEdit, 1, 1);
	filterLayout->addWidget(maxButton, 1, 2);
	maxEdit = new EqonomizeValueEdit(false, this);
	maxEdit->setEnabled(false);
	QSizePolicy sp = maxEdit->sizePolicy();
	sp.setHorizontalPolicy(QSizePolicy::Expanding);
	maxEdit->setSizePolicy(sp);
	filterLayout->addWidget(maxEdit, 1, 3);
	filterLayout->addWidget(new QLabel(i18n("Description:"), this), 3, 0);
	descriptionEdit = new QLineEdit(this);
	descriptionEdit->setCompleter(new QCompleter(this));
	descriptionEdit->completer()->setModel(new QStandardItemModel(this));
	descriptionEdit->completer()->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	descriptionEdit->completer()->setCaseSensitivity(Qt::CaseInsensitive);
	filterLayout->addWidget(descriptionEdit, 3, 1);
	if(b_extra && (transtype == TRANSACTION_TYPE_EXPENSE || transtype == TRANSACTION_TYPE_INCOME)) {
		if(transtype == TRANSACTION_TYPE_INCOME) filterLayout->addWidget(new QLabel(i18n("Payer:"), this), 3, 2);
		else filterLayout->addWidget(new QLabel(i18n("Payee:"), this), 3, 2);
		payeeEdit = new QLineEdit(this);
		payeeEdit->setCompleter(new QCompleter(this));
		payeeEdit->completer()->setModel(new QStandardItemModel(this));
		payeeEdit->completer()->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
		payeeEdit->completer()->setCaseSensitivity(Qt::CaseInsensitive);
		filterLayout->addWidget(payeeEdit, 3, 3);
	}
	QHBoxLayout *filterExcludeLayout = new QHBoxLayout();
	group = new QButtonGroup(this);
	includeButton = new QRadioButton(i18n("Include"), this);
	includeButton->setChecked(true);
	group->addButton(includeButton);
	filterExcludeLayout->addWidget(includeButton);
	excludeButton = new QRadioButton(i18n("Exclude"), this);
	group->addButton(excludeButton);
	filterExcludeLayout->addWidget(excludeButton);
	exactMatchButton = new QCheckBox(i18n("Exact match"), this);
	filterExcludeLayout->addWidget(exactMatchButton);
	filterExcludeLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
	clearButton = new QPushButton(this);
	KGuiItem::assign(clearButton, KStandardGuiItem::clear());
	clearButton->setEnabled(false);
	filterExcludeLayout->addWidget(clearButton);
	if(payeeEdit) {
		filterLayout->addLayout(filterExcludeLayout, 4, 0, 1, 4);
		filterLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding), 5, 0, 1, 4);
	} else {
		filterLayout->addLayout(filterExcludeLayout, 3, 2, 1, 2);
		filterLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0, 1, 4);
	}

	fromCombo->addItem(i18n("All"));
	toCombo->addItem(i18n("All"));

	if(payeeEdit) {
		connect(payeeEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(filter()));
		connect(payeeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkEnableClear()));
	}
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clearFilter()));
	connect(group, SIGNAL(buttonClicked(int)), this, SIGNAL(filter()));
	connect(dateFromButton, SIGNAL(toggled(bool)), dateFromEdit, SLOT(setEnabled(bool)));
	connect(dateFromButton, SIGNAL(toggled(bool)), this, SIGNAL(filter()));
	connect(dateFromButton, SIGNAL(toggled(bool)), this, SLOT(checkEnableClear()));
	connect(dateFromEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(fromChanged(const QDate&)));
	connect(dateToEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(toChanged(const QDate&)));
	connect(dateToEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkEnableClear()));
	connect(toCombo, SIGNAL(activated(int)), this, SIGNAL(filter()));
	connect(fromCombo, SIGNAL(activated(int)), this, SIGNAL(filter()));
	connect(toCombo, SIGNAL(activated(int)), this, SLOT(checkEnableClear()));
	connect(fromCombo, SIGNAL(activated(int)), this, SLOT(checkEnableClear()));
	connect(toCombo, SIGNAL(activated(int)), this, SIGNAL(toActivated(int)));
	connect(fromCombo, SIGNAL(activated(int)), this, SIGNAL(fromActivated(int)));
	connect(descriptionEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(filter()));
	connect(descriptionEdit, SIGNAL(textChanged(const QString&)), this, SLOT(checkEnableClear()));
	connect(minButton, SIGNAL(toggled(bool)), this, SIGNAL(filter()));
	connect(minButton, SIGNAL(toggled(bool)), this, SLOT(checkEnableClear()));
	connect(minButton, SIGNAL(toggled(bool)), minEdit, SLOT(setEnabled(bool)));
	connect(minEdit, SIGNAL(valueChanged(double)), this, SIGNAL(filter()));
	connect(maxButton, SIGNAL(toggled(bool)), this, SIGNAL(filter()));
	connect(maxButton, SIGNAL(toggled(bool)), this, SLOT(checkEnableClear()));
	connect(maxButton, SIGNAL(toggled(bool)), maxEdit, SLOT(setEnabled(bool)));
	connect(maxEdit, SIGNAL(valueChanged(double)), this, SIGNAL(filter()));
	connect(exactMatchButton, SIGNAL(toggled(bool)), this, SIGNAL(filter()));

}

TransactionFilterWidget::~TransactionFilterWidget() {
	delete group;
}

void TransactionFilterWidget::currentDateChanged(const QDate &olddate, const QDate &newdate) {
	if(olddate == to_date) {
		dateToEdit->blockSignals(true);
		dateToEdit->setDate(newdate);
		dateToEdit->blockSignals(false);
		toChanged(newdate);
	}
}

void TransactionFilterWidget::clearFilter() {
	dateFromButton->blockSignals(true);
	dateToEdit->blockSignals(true);
	minButton->blockSignals(true);
	maxButton->blockSignals(true);
	fromCombo->blockSignals(true);
	toCombo->blockSignals(true);
	descriptionEdit->blockSignals(true);
	if(payeeEdit) payeeEdit->blockSignals(true);
	dateFromButton->setChecked(false);
	dateFromEdit->setEnabled(false);
	to_date = QDate::currentDate();
	dateToEdit->setDate(to_date);
	minButton->setChecked(false);
	minEdit->setEnabled(false);
	maxButton->setChecked(false);
	maxEdit->setEnabled(false);
	fromCombo->setCurrentIndex(0);
	toCombo->setCurrentIndex(0);
	descriptionEdit->setText(QString::null);
	if(payeeEdit) payeeEdit->setText(QString::null);
	dateFromButton->blockSignals(false);
	dateToEdit->blockSignals(false);
	minButton->blockSignals(false);
	maxButton->blockSignals(false);
	fromCombo->blockSignals(false);
	toCombo->blockSignals(false);
	descriptionEdit->blockSignals(false);
	if(payeeEdit) payeeEdit->blockSignals(false);
	clearButton->setEnabled(false);
	emit filter();
}
void TransactionFilterWidget::checkEnableClear() {
	clearButton->setEnabled(dateFromButton->isChecked() || minButton->isChecked() || maxButton->isChecked() || fromCombo->currentIndex() || toCombo->currentIndex() || !descriptionEdit->text().isEmpty() || (payeeEdit && !payeeEdit->text().isEmpty()) || to_date != QDate::currentDate());
}
void TransactionFilterWidget::setFilter(QDate fromdate, QDate todate, double min, double max, Account *from_account, Account *to_account, QString description, QString payee, bool exclude, bool exact_match) {
	dateFromButton->blockSignals(true);
	dateFromEdit->blockSignals(true);
	dateToEdit->blockSignals(true);
	minButton->blockSignals(true);
	maxButton->blockSignals(true);
	minEdit->blockSignals(true);
	maxEdit->blockSignals(true);
	fromCombo->blockSignals(true);
	toCombo->blockSignals(true);
	descriptionEdit->blockSignals(true);
	if(payeeEdit) payeeEdit->blockSignals(true);
	excludeButton->blockSignals(true);
	includeButton->blockSignals(true);
	exactMatchButton->blockSignals(true);
	dateToEdit->setDate(todate);
	to_date = todate;
	if(fromdate.isNull()) {
		dateFromButton->setChecked(false);
		dateFromEdit->setEnabled(false);
		if(dateFromEdit->date() > todate) {
			dateFromEdit->setDate(todate);
			from_date = todate;
		}
	} else {
		dateFromButton->setChecked(true);
		dateFromEdit->setEnabled(true);
		dateFromEdit->setDate(fromdate);
		from_date = fromdate;
	}
	if(min < 0.0) {
		minButton->setChecked(false);
		minEdit->setEnabled(false);
	} else {
		minButton->setChecked(true);
		minEdit->setEnabled(true);
		minEdit->setValue(min);
	}
	if(max < 0.0) {
		maxButton->setChecked(false);
		maxEdit->setEnabled(false);
	} else {
		maxButton->setChecked(true);
		maxEdit->setEnabled(true);
		maxEdit->setValue(max);
	}
	if(from_account) {
		for(QVector<Account*>::size_type i = 0; i < froms.size(); i++) {
			if(froms[i] == from_account) {
				fromCombo->setCurrentIndex(i + 1);
				emit fromActivated(i + 1);
				break;
			}
		}
	} else {
		fromCombo->setCurrentIndex(0);
		emit fromActivated(0);
	}
	if(to_account) {
		for(QVector<Account*>::size_type i = 0; i < tos.size(); i++) {
			if(tos[i] == to_account) {
				toCombo->setCurrentIndex(i + 1);
				emit toActivated(i + 1);
				break;
			}
		}
	} else {
		toCombo->setCurrentIndex(0);
		emit toActivated(0);
	}
	descriptionEdit->setText(description);
	if(payeeEdit) descriptionEdit->setText(payee);
	excludeButton->setChecked(exclude);
	exactMatchButton->setChecked(exact_match);
	checkEnableClear();
	dateFromButton->blockSignals(false);
	dateFromEdit->blockSignals(false);
	dateToEdit->blockSignals(false);
	minButton->blockSignals(false);
	maxButton->blockSignals(false);
	minEdit->blockSignals(false);
	maxEdit->blockSignals(false);
	fromCombo->blockSignals(false);
	toCombo->blockSignals(false);
	descriptionEdit->blockSignals(false);
	if(payeeEdit) payeeEdit->blockSignals(false);
	excludeButton->blockSignals(false);
	includeButton->blockSignals(false);
	exactMatchButton->blockSignals(false);
	emit filter();
}
void TransactionFilterWidget::updateFromAccounts() {
	fromCombo->clear();
	froms.clear();
	fromCombo->addItem(i18n("All"));
	Account *account;
	switch(transtype) {
		case TRANSACTION_TYPE_TRANSFER: {
			account = budget->assetsAccounts.first();
			while(account) {
				fromCombo->addItem(account->name());
				froms.push_back(account);
				account = budget->assetsAccounts.next();
			}
			break;
		}
		case TRANSACTION_TYPE_EXPENSE: {
			account = budget->assetsAccounts.first();
			while(account) {
				if(account != budget->balancingAccount && ((AssetsAccount*) account)->accountType() != ASSETS_TYPE_SECURITIES) {
					fromCombo->addItem(account->name());
					froms.push_back(account);
				}
				account = budget->assetsAccounts.next();
			}
			break;
		}
		case TRANSACTION_TYPE_INCOME: {
			account = budget->incomesAccounts.first();
			while(account) {
				fromCombo->addItem(account->name());
				froms.push_back(account);
				account = budget->incomesAccounts.next();
			}
			break;
		}
	}
}
void TransactionFilterWidget::updateToAccounts() {
	toCombo->clear();
	tos.clear();
	toCombo->addItem(i18n("All"));
	Account *account;
	switch(transtype) {
		case TRANSACTION_TYPE_TRANSFER: {
			account = budget->assetsAccounts.first();
			while(account) {
				toCombo->addItem(account->name());
				tos.push_back(account);
				account = budget->assetsAccounts.next();
			}
			break;
		}
		case TRANSACTION_TYPE_INCOME: {
			account = budget->assetsAccounts.first();
			while(account) {
				if(account != budget->balancingAccount) {
					toCombo->addItem(account->name());
					tos.push_back(account);
				}
				account = budget->assetsAccounts.next();
			}
			break;
		}
		case TRANSACTION_TYPE_EXPENSE: {
			account = budget->expensesAccounts.first();
			while(account) {
				toCombo->addItem(account->name());
				tos.push_back(account);
				account = budget->expensesAccounts.next();
			}
			break;
		}
	}
}
void TransactionFilterWidget::updateAccounts() {
	updateFromAccounts();
	updateToAccounts();
}
bool TransactionFilterWidget::filterTransaction(Transaction *trans, bool checkdate) {
	if(trans->type() == TRANSACTION_TYPE_SECURITY_BUY || trans->type() == TRANSACTION_TYPE_SECURITY_SELL) {
		if(transtype == TRANSACTION_TYPE_TRANSFER && ((SecurityTransaction*) trans)->account()->type() != ACCOUNT_TYPE_ASSETS) return true;
		if(transtype == TRANSACTION_TYPE_EXPENSE && ((SecurityTransaction*) trans)->account()->type() != ACCOUNT_TYPE_EXPENSES) return true;
		if(transtype == TRANSACTION_TYPE_INCOME && ((SecurityTransaction*) trans)->account()->type() != ACCOUNT_TYPE_INCOMES) return true;
	} else if(trans->type() != transtype) {
		return true;
	}
	if(includeButton->isChecked()) {
		if(toCombo->currentIndex() > 0 && tos[toCombo->currentIndex() - 1] != trans->toAccount()) {
			return true;
		}
		if(fromCombo->currentIndex() > 0 && froms[fromCombo->currentIndex() - 1] != trans->fromAccount()) {
			return true;
		}
		if(exactMatchButton->isChecked()) {
			if(!descriptionEdit->text().isEmpty() && trans->description().compare(descriptionEdit->text(), Qt::CaseInsensitive) != 0) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_EXPENSE && !payeeEdit->text().isEmpty() && ((Expense*) trans)->payee().compare(payeeEdit->text(), Qt::CaseInsensitive) != 0) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_INCOME && !payeeEdit->text().isEmpty() && ((Income*) trans)->payer().compare(payeeEdit->text(), Qt::CaseInsensitive) != 0) {
				return true;
			}
		} else {
			if(!descriptionEdit->text().isEmpty() && !trans->description().contains(descriptionEdit->text(), Qt::CaseInsensitive)) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_EXPENSE && !payeeEdit->text().isEmpty() && !((Expense*) trans)->payee().contains(payeeEdit->text(), Qt::CaseInsensitive)) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_INCOME && !payeeEdit->text().isEmpty() && !((Income*) trans)->payer().contains(payeeEdit->text(), Qt::CaseInsensitive)) {
				return true;
			}
		}
	} else {
		if(toCombo->currentIndex() > 0 && tos[toCombo->currentIndex() - 1] == trans->toAccount()) {
			return true;
		}
		if(fromCombo->currentIndex() > 0 && froms[fromCombo->currentIndex() - 1] == trans->fromAccount()) {
			return true;
		}
		if(exactMatchButton->isChecked()) {
			if(!descriptionEdit->text().isEmpty() && trans->description().compare(descriptionEdit->text(), Qt::CaseInsensitive) == 0) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_EXPENSE  && !payeeEdit->text().isEmpty() && ((Expense*) trans)->payee().compare(payeeEdit->text(), Qt::CaseInsensitive) == 0) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_INCOME  && !payeeEdit->text().isEmpty() && ((Income*) trans)->payer().compare(payeeEdit->text(), Qt::CaseInsensitive) == 0) {
				return true;
			}
		} else {
			if(!descriptionEdit->text().isEmpty() && trans->description().contains(descriptionEdit->text(), Qt::CaseInsensitive)) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_EXPENSE  && !payeeEdit->text().isEmpty() && ((Expense*) trans)->payee().contains(payeeEdit->text(), Qt::CaseInsensitive)) {
				return true;
			}
			if(payeeEdit && transtype == TRANSACTION_TYPE_INCOME  && !payeeEdit->text().isEmpty() && ((Income*) trans)->payer().contains(payeeEdit->text(), Qt::CaseInsensitive)) {
				return true;
			}
		}
	}
	if(minButton->isChecked() && trans->value() < minEdit->value()) {
		return true;
	}
	if(maxButton->isChecked() && trans->value() > maxEdit->value()) {
		return true;
	}
	if(checkdate && dateFromButton->isChecked() && trans->date() < from_date) {
		return true;
	}
	if(checkdate && trans->date() > to_date) {
		return true;
	}
	return false;
}
QDate TransactionFilterWidget::startDate() {
	if(!dateFromButton->isChecked()) return QDate();
	return from_date;
}
QDate TransactionFilterWidget::endDate() {
	return to_date;
}
void TransactionFilterWidget::transactionsReset() {
	((QStandardItemModel*) descriptionEdit->completer()->model())->clear();
	if(payeeEdit) ((QStandardItemModel*) payeeEdit->completer()->model())->clear();
	QStringList payee_list;
	QStringList descr_list;
	switch(transtype) {
		case TRANSACTION_TYPE_EXPENSE: {
			Expense *expense = budget->expenses.last();
			while(expense) {
				if(!expense->description().isEmpty() && !descr_list.contains(expense->description(), Qt::CaseInsensitive)) {
					QList<QStandardItem*> row;
					row << new QStandardItem(expense->description());
					row << new QStandardItem(expense->description().toLower());
					((QStandardItemModel*) descriptionEdit->completer()->model())->appendRow(row);
					descr_list << expense->description().toLower();
				}
				if(payeeEdit && !expense->payee().isEmpty() && !payee_list.contains(expense->payee(), Qt::CaseInsensitive)) {
					QList<QStandardItem*> row;
					row << new QStandardItem(expense->payee());
					row << new QStandardItem(expense->payee().toLower());
					((QStandardItemModel*) payeeEdit->completer()->model())->appendRow(row);
					payee_list << expense->payee().toLower();
				}
				expense = budget->expenses.previous();
			}
			break;
		}
		case TRANSACTION_TYPE_INCOME: {
			Income *income = budget->incomes.last();
			while(income) {
				if(!income->security() && !income->description().isEmpty() && !descr_list.contains(income->description(), Qt::CaseInsensitive)) {
					QList<QStandardItem*> row;
					row << new QStandardItem(income->description());
					row << new QStandardItem(income->description().toLower());
					((QStandardItemModel*) descriptionEdit->completer()->model())->appendRow(row);
					descr_list << income->description().toLower();
				}
				if(payeeEdit && !income->security() && !income->payer().isEmpty() && !payee_list.contains(income->payer(), Qt::CaseInsensitive)) {
					QList<QStandardItem*> row;
					row << new QStandardItem(income->payer());
					row << new QStandardItem(income->payer().toLower());
					((QStandardItemModel*) payeeEdit->completer()->model())->appendRow(row);
					payee_list << income->payer().toLower();
				}
				income = budget->incomes.previous();
			}
			break;
		}
		case TRANSACTION_TYPE_TRANSFER: {
			Transfer *transfer= budget->transfers.last();
			while(transfer) {
				if(!transfer->description().isEmpty() && !descr_list.contains(transfer->description(), Qt::CaseInsensitive)) {
					QList<QStandardItem*> row;
					row << new QStandardItem(transfer->description());
					row << new QStandardItem(transfer->description().toLower());
					((QStandardItemModel*) descriptionEdit->completer()->model())->appendRow(row);
					descr_list << transfer->description().toLower();
				}
				transfer = budget->transfers.previous();
			}
			break;
		}
		default: {}
	}
	((QStandardItemModel*) descriptionEdit->completer()->model())->sort(1);
	if(payeeEdit) ((QStandardItemModel*) payeeEdit->completer()->model())->sort(1);
}
void TransactionFilterWidget::transactionAdded(Transaction *trans) {
	if(descriptionEdit && trans->type() == transtype && (transtype != TRANSACTION_TYPE_INCOME || !((Income*) trans)->security())) {
		if(!trans->description().isEmpty()) {
			if(((QStandardItemModel*) descriptionEdit->completer()->model())->findItems(trans->description().toLower(), Qt::MatchExactly, 1).isEmpty()) {
				QList<QStandardItem*> row;
				row << new QStandardItem(trans->description());
				row << new QStandardItem(trans->description().toLower());
				((QStandardItemModel*) descriptionEdit->completer()->model())->appendRow(row);
				((QStandardItemModel*) descriptionEdit->completer()->model())->sort(1);
			}
		}
		if(payeeEdit && transtype == TRANSACTION_TYPE_EXPENSE && !((Expense*) trans)->payee().isEmpty()) {
			if(((QStandardItemModel*) payeeEdit->completer()->model())->findItems(((Expense*) trans)->payee().toLower(), Qt::MatchExactly, 1).isEmpty()) {
				QList<QStandardItem*> row;
				row << new QStandardItem(((Expense*) trans)->payee());
				row << new QStandardItem(((Expense*) trans)->payee().toLower());
				((QStandardItemModel*) payeeEdit->completer()->model())->appendRow(row);
				((QStandardItemModel*) payeeEdit->completer()->model())->sort(1);
			}
		} else if(payeeEdit && transtype == TRANSACTION_TYPE_INCOME && !((Income*) trans)->security() && !((Income*) trans)->payer().isEmpty()) {
			if(((QStandardItemModel*) payeeEdit->completer()->model())->findItems(((Income*) trans)->payer().toLower(), Qt::MatchExactly, 1).isEmpty()) {
				QList<QStandardItem*> row;
				row << new QStandardItem(((Income*) trans)->payer());
				row << new QStandardItem(((Income*) trans)->payer().toLower());
				((QStandardItemModel*) payeeEdit->completer()->model())->appendRow(row);
				((QStandardItemModel*) payeeEdit->completer()->model())->sort(1);
			}
		}
	}
}
void TransactionFilterWidget::transactionModified(Transaction *trans) {
	transactionAdded(trans);
}
void TransactionFilterWidget::toChanged(const QDate &date) {
	bool error = false;
	if(!date.isValid()) {
		QMessageBox::critical(this, i18n("Error"), i18n("Invalid date."));
		error = true;
	}
	if(!error && dateFromEdit->date() > date) {
		if(dateFromButton->isChecked()) {
			QMessageBox::critical(this, i18n("Error"), i18n("To date is before from date."));
		}
		from_date = date;
		dateFromEdit->blockSignals(true);
		dateFromEdit->setDate(from_date);
		dateFromEdit->blockSignals(false);
	}
	if(error) {
		dateToEdit->setFocus();
		dateToEdit->blockSignals(true);
		dateToEdit->setDate(to_date);
		dateToEdit->blockSignals(false);
		dateToEdit->selectAll();
		return;
	}
	to_date = date;
	emit filter();
}
void TransactionFilterWidget::fromChanged(const QDate &date) {
	bool error = false;
	if(!date.isValid()) {
		QMessageBox::critical(this, i18n("Error"), i18n("Invalid date."));
		error = true;
	}
	if(!error && date > dateToEdit->date()) {
		QMessageBox::critical(this, i18n("Error"), i18n("From date is after to date."));
		to_date = date;
		dateToEdit->blockSignals(true);
		dateToEdit->setDate(to_date);
		dateToEdit->blockSignals(false);
	}
	if(error) {
		dateFromEdit->setFocus();
		dateFromEdit->blockSignals(true);
		dateFromEdit->setDate(from_date);
		dateFromEdit->blockSignals(false);
		dateFromEdit->selectAll();
		return;
	}
	from_date = date;
	if(dateFromButton->isChecked()) emit filter();
}
double TransactionFilterWidget::countYears() {
	QDate first_date = firstDate();
	if(first_date.isNull()) return 0.0;
	return yearsBetweenDates(first_date, to_date);
}
double TransactionFilterWidget::countMonths() {
	QDate first_date = firstDate();
	if(first_date.isNull()) return 0.0;
	return monthsBetweenDates(first_date, to_date);
}
int TransactionFilterWidget::countDays() {
	QDate first_date = firstDate();
	if(first_date.isNull()) return 0;
	return first_date.daysTo(to_date) + 1;
}
QDate TransactionFilterWidget::firstDate() {
	if(dateFromButton->isChecked()) return from_date;
	QDate first_date;
	switch(transtype) {
		case TRANSACTION_TYPE_EXPENSE: {
			if(!budget->expenses.isEmpty()) first_date = budget->expenses.getFirst()->date();
			SecurityTransaction *sectrans = budget->securityTransactions.first();
			while(sectrans) {
				if(!first_date.isNull() && sectrans->date() >= first_date) break;
				if(sectrans->account()->type() == ACCOUNT_TYPE_EXPENSES) {
					first_date = sectrans->date();
					break;
				}
				sectrans = budget->securityTransactions.next();
			}
			if(first_date.isNull()) {
				ScheduledTransaction *strans = budget->scheduledTransactions.first();
				while(strans) {
					if(strans->transaction()->type() == TRANSACTION_TYPE_EXPENSE || ((strans->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || strans->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY) && ((SecurityTransaction*) strans->transaction())->account()->type() == ACCOUNT_TYPE_EXPENSES)) {
						first_date = strans->transaction()->date();
						break;
					}
					strans = budget->scheduledTransactions.next();
				}
			}
			break;
		}
		case TRANSACTION_TYPE_INCOME: {
			if(!budget->incomes.isEmpty()) first_date = budget->incomes.getFirst()->date();
			SecurityTransaction *sectrans = budget->securityTransactions.first();
			while(sectrans) {
				if(!first_date.isNull() && sectrans->date() >= first_date) break;
				if(sectrans->account()->type() == ACCOUNT_TYPE_INCOMES) {
					first_date = sectrans->date();
					break;
				}
				sectrans = budget->securityTransactions.next();
			}
			if(first_date.isNull()) {
				ScheduledTransaction *strans = budget->scheduledTransactions.first();
				while(strans) {
					if(strans->transaction()->type() == TRANSACTION_TYPE_INCOME || ((strans->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || strans->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY) && ((SecurityTransaction*) strans->transaction())->account()->type() == ACCOUNT_TYPE_INCOMES)) {
						first_date = strans->transaction()->date();
						break;
					}
					strans = budget->scheduledTransactions.next();
				}
			}
			break;
		}
		case TRANSACTION_TYPE_TRANSFER: {
			if(!budget->transfers.isEmpty()) first_date = budget->transfers.getFirst()->date();
			SecurityTransaction *sectrans = budget->securityTransactions.first();
			while(sectrans) {
				if(!first_date.isNull() && sectrans->date() >= first_date) break;
				if(sectrans->account()->type() == ACCOUNT_TYPE_ASSETS) {
					first_date = sectrans->date();
					break;
				}
				sectrans = budget->securityTransactions.next();
			}
			if(first_date.isNull()) {
				ScheduledTransaction *strans = budget->scheduledTransactions.first();
				while(strans) {
					if(strans->transaction()->type() == TRANSACTION_TYPE_TRANSFER || ((strans->transaction()->type() == TRANSACTION_TYPE_SECURITY_SELL || strans->transaction()->type() == TRANSACTION_TYPE_SECURITY_BUY) && ((SecurityTransaction*) strans->transaction())->account()->type() == ACCOUNT_TYPE_ASSETS)) {
						first_date = strans->transaction()->date();
						break;
					}
					strans = budget->scheduledTransactions.next();
				}
			}
			break;
		}
		default: {break;}
	}
	return first_date;
}

#include "transactionfilterwidget.moc"

