#include "yeardao.h"

YearDAO::YearDAO(QObject *parent) :
	GenericDAO(parent, GenericDAO::Year)
{
}

YearDAO::YearDAO(const YearDAO &other) :
	GenericDAO(other)
{
	_year = other.year();
}

YearDAO::~YearDAO()
{}

QString YearDAO::year() const { return _year; }
void YearDAO::setYear(const QString &year) { _year = year; }
