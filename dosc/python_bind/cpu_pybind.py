# plugins/cpu/cpu_pybind.py
import json, re, time

CPU_LINE = re.compile(r'^(cpu\d*?)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)(?:\s+(\d+)\s+(\d+)\s+(\d+))?')

def _read_proc_stat():
    totals = {}
    procs_running = procs_blocked = None
    with open("/proc/stat", "r") as f:
        for line in f:
            if line.startswith("cpu"):
                m = CPU_LINE.match(line)
                if not m: continue
                name = m.group(1)
                user, nice, system, idle = map(int, m.group(2,3,4,5))
                iowait = int(m.group(6) or 0)
                irq    = int(m.group(7) or 0)
                soft   = int(m.group(8) or 0)
                totals[name] = dict(user=user, nice=nice, system=system, idle=idle,
                                    iowait=iowait, irq=irq, softirq=soft, steal=0)
            elif line.startswith("procs_running"):
                procs_running = int(line.split()[1])
            elif line.startswith("procs_blocked"):
                procs_blocked = int(line.split()[1])
    return totals, procs_running, procs_blocked

def _read_loadavg():
    with open("/proc/loadavg","r") as f:
        a,b,c,_ = f.read().split(maxsplit=3)
        return float(a), float(b), float(c)

def _usage(prev,cur):
    out={}
    for cpu in cur:
        if cpu not in prev: continue
        a,b = prev[cpu], cur[cpu]
        idle_a = a["idle"] + a["iowait"]
        idle_b = b["idle"] + b["iowait"]
        non_a  = a["user"]+a["nice"]+a["system"]+a["irq"]+a["softirq"]+a["steal"]
        non_b  = b["user"]+b["nice"]+b["system"]+b["irq"]+b["softirq"]+b["steal"]
        totald = (idle_b-idle_a) + (non_b-non_a)
        if totald <= 0: continue
        used = totald - (idle_b-idle_a)
        out[cpu] = dict(
            usage = 100.0*used/totald,
            iowait= 100.0*(b["iowait"]-a["iowait"])/totald,
            steal = 0.0
        )
    return out

def collect(ctx):
    # ctx може бути dict або JSON-рядок — підтримаймо обидва
    if isinstance(ctx, str):
        try: ctx = json.loads(ctx) if ctx else {}
        except Exception: ctx = {}
    elif not isinstance(ctx, dict):
        ctx = {}

    a, pr_a, pb_a = _read_proc_stat()
    time.sleep(0.15)
    b, pr_b, pb_b = _read_proc_stat()
    l1,l5,l15 = _read_loadavg()

    uses = _usage(a,b)
    metrics = []
    for cpu, v in uses.items():
        metrics.append({"name":"cpu_usage_percent","type":"gauge","value":v["usage"],"labels":{"cpu":cpu}})
        metrics.append({"name":"cpu_iowait_percent","type":"gauge","value":v["iowait"],"labels":{"cpu":cpu}})
    if pr_b is not None: metrics.append({"name":"procs_running","type":"gauge","value":float(pr_b),"labels":{}})
    if pb_b is not None: metrics.append({"name":"procs_blocked","type":"gauge","value":float(pb_b),"labels":{}})
    metrics += [
        {"name":"load1","type":"gauge","value":l1,"labels":{}},
        {"name":"load5","type":"gauge","value":l5,"labels":{}},
        {"name":"load15","type":"gauge","value":l15,"labels":{}},
    ]
    
    print(metrics)
    
    return {"metrics": metrics}
