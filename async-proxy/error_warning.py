#encoding:utf-8
from email import encoders
from email.header import Header
from email.utils import parseaddr, formataddr
from email.mime.text import MIMEText
import string
import time
import smtplib
import re
import os
import types

err_type = {
		"DB_ERR":"数据库错误",
		"NET_ERR":"网络错误",
		"SYS_ERR":"系统调用错误",
		"LOGI_ERR":"程序逻辑错误",
		"EXCE_ERR":"程序异常错误"
		}

def _format_addr(s):
	name, addr = parseaddr(s)
	return formataddr(( \
			Header(name, 'utf-8').encode(), \
			addr.encode('utf-8') if isinstance(addr, unicode) else addr))

def _send_mail(message, addr):
	addr = addr.strip('\0')
	if message == None:
		return None
	if addr == None:
		return None

	from_addr = 'boss_monitor@taomee.com'
	to_addr = addr
	password = ''
	smtp_server = 'mail.shidc.taomee.com'

	msg = MIMEText(message,'html','utf-8')
	msg['From'] = _format_addr(u'boss_monitor <%s>' % from_addr)
	msg['To'] = _format_addr(u'管理员 <%s>' % to_addr)
	msg['Subject'] = Header(u'BOSS服务端异常', 'utf-8').encode()
	server = smtplib.SMTP(smtp_server, 25)
	server.set_debuglevel(1)
	server.sendmail(from_addr, [to_addr], msg.as_string())
	server.quit()

#取得十分钟前的日期
def GetDate(file_name):
	f = os.popen('cd ./log && ls -ltr | grep %s | awk \'{print $6}\'' % file_name)
	if not f:
		return None

	date = f.readlines()
	if len(date) != 1:
		return None
	else:
		return date[0].strip('\n')

def GetTimeStamp(a):
	if a == 'now':
		return int(time.time())
	else:
		timeArray = time.strptime(a,'%Y-%m-%d %H:%M:%S')
		return int(time.mktime(timeArray))

def GetSerName():
	f = os.popen('sh server.sh state | grep MAIN | awk \'{print $8}\' | awk -F\'-[MAIN]\' \'{print $1}\'')
	if not f:
		return None

	ser_name = f.readlines()
	return ser_name[0].strip('\n')

def GetLog(min):
	log_names = os.popen('find ./log -cmin -%s | grep crit | grep -v .swp| awk -F\'/\' \'{print $3}\'' % min)
	if not log_names:
		return None

	info_list = []
	for log_file in log_names.readlines():
		fd = open("./log/" + log_file.strip('\n'), 'r')
		date = GetDate(log_file.strip('\n'))
		if not date:
			return None

		for line in fd.readlines():
			log_info = {}
			tmp_info = re.split(']|\[', line.strip('\n'))
			if len(tmp_info) == 0:
				return None

			log_info['time'] = date + ' ' + tmp_info[1]
			now = GetTimeStamp('now') - (min*60)
			time = GetTimeStamp(log_info['time'])
			if time < now:
				continue;

			log_info['name'] = GetSerName()
			if not log_info['name']:
				return None

			if tmp_info[9]:
				log_info['error_type'] = err_type[tmp_info[9]]
				n = 10
			else:
				n = 8

			log_info['content'] = ""
			i = 0
			for content in tmp_info:
				if i < n:
					i += 1
					continue
				log_info['content'] += content
			info_list.append(log_info)
	return info_list


def GetMessage(min):
	message = '<h1>发现错误，错误内容如下</h1><table border="1" cellpadding="4"><tr><td>系统名</td><td>错误类型</td><td>错误时间</td><td width="60%">错误内容</td></tr>'
	info_list = GetLog(10)
	if len(info_list) == 0:
		return None

	i = 1
	for info in info_list:
		s = '<tr><td>' + info['name'] +'</td><td>' + info['error_type'] + '</td><td>' + info['time'] + '</td><td width="60%">' + info['content'] + '</td></tr>'
		message += s
		i += 1
		if i > 20:
			break
	message += '</table>'

	if i < len(info_list):
		message += '<br /><br /><font color="red">总共找到%d行错误，以上只显示20条，其余被省略，请自行查看！</font>' % len(info_list)
	return message

def SendMail(message, addr):
	if type(addr) != list:
		return None

	for a in addr:
		_send_mail(message, a)

import get_addr_list
def GetAddrList():
	IP = 'test.service-config.taomee.com'
	PORT = 19155
	return get_addr_list.unpack(get_addr_list.send_data(get_addr_list.pack(), IP, PORT))

#---------main()--------------#
import sys
args = sys.argv
if len(args) != 2:
	quit()

f = os.popen('grep use_monitor conf/bench.conf  | grep 1')
if not f:
	quit()
if len(f.readlines()) == 0:
	quit()

addr = GetAddrList()
#SendMail(GetMessage(10), addr)
SendMail(GetMessage(args[0]), addr)
