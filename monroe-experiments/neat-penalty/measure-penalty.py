#/usr/bin/python
from datetime import datetime
from time import sleep
import psutil

def format_tool(tool, data, data_prev):
  if tool in data and tool in data_prev:
    cpu = data[tool]['cpu']
    mem = data[tool]['mem']
    return '\t%s\t%f\t%d' % (tool, cpu, mem.pss)
  else:
    return '\t\t\t'

def main():

  data_prev = {}
  print("DTM\tCPU\tTOTAL\tAVAIL\tNAME\tCPU\tPSS\tNAME\tCPU\tPSS\tNAME\tCPU\tPSS\tNAME\tCPU\tPSS")

  while True:
    dtm = datetime.now()
    cpu = psutil.cpu_percent()
    mem = psutil.virtual_memory()

    data = {}
    tools = ['tcp-ping', 'neat-tcp-ping', 'dwnl-test', 'neat-dwnl-test', 
             'neat-proxy', 'neat-metadata-exporter']
    for p in psutil.process_iter():
      try:
        if p.name() in tools:
          data[p.name()] = {'cpu': p.cpu_percent(), 'mem': p.memory_full_info()}
        elif '/opt/celerway/neat/policy/neatpmd' in p.cmdline():
          data['neatpmd'] = {'cpu': p.cpu_percent(), 'mem': p.memory_full_info()}
      except psutil.NoSuchProcess:
        pass
    
    outstr = "%s\t%f\t%d\t%d" % (dtm.strftime('%H:%M:%S.%f'),
      cpu, mem.total, mem.available)

    if 'tcp-ping' in data:
      outstr += format_tool('tcp-ping', data, data_prev)
    elif 'neat-tcp-ping' in data:
      outstr += format_tool('neat-tcp-ping', data, data_prev)
    elif 'dwnl-test' in data:
      outstr += format_tool('dwnl-test', data, data_prev)
    elif 'neat-dwnl-test' in data:
      outstr += format_tool('neat-dwnl-test', data, data_prev)
    else:
      outstr += '\t(no-app)\t\t'

    outstr += format_tool('neat-metadata-exporter', data, data_prev)
    outstr += format_tool('neatpmd', data, data_prev)
    outstr += format_tool('neat-proxy', data, data_prev)

    print(outstr)

    data_prev = data

    sleep(5)

if __name__ == '__main__':
  main()
