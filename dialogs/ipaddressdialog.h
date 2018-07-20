/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
#ifndef IPADDRESSDIALOG_H
#define IPADDRESSDIALOG_H

#include <QDialog>

namespace Ui
{
    class IpAddressDialog;
}

class IpAddressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IpAddressDialog(QWidget *parent = 0); // the QWidget will be a window bc *parent=0
    ~IpAddressDialog();

    QString getIpAddress();
    QString getPort();

private:
    Ui::IpAddressDialog *ui;
};

#endif // IPADDRESSDIALOG_H
