# #!/usr/bin/env python3
# import sys, json, time
# ctx = json.loads(sys.stdin.read() or "{}")
# # ... читаємо логи/системні дані
# out = {"metrics":[
#   {"name":"pos_transactions_total","type":"counter","value":1234,
#    "labels":{"store_id": ctx.get("store_id","unknown")}}
# ]}
# print(json.dumps(out))

#!/usr/bin/env python3
import sys, json, time, re

CPU_LINE = re.compile(r'^(cpu\d*?)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)'
                      r'(?:\s+(\d+)\s+(\d+)\s+(\d+))?')

def read_proc_stat():
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
                steal  = 0  # спростимо; можна дочитати далі поля
                totals[name] = {
                    "user":user, "nice":nice, "system":system, "idle":idle,
                    "iowait":iowait, "irq":irq, "softirq":soft, "steal":steal
                }
            elif line.startswith("procs_running"):
                procs_running = int(line.split()[1])
            elif line.startswith("procs_blocked"):
                procs_blocked = int(line.split()[1])
    return totals, procs_running, procs_blocked

def read_loadavg():
    with open("/proc/loadavg","r") as f:
        a,b,c,_ = f.read().split(maxsplit=3)
        return float(a), float(b), float(c)

def usage_from_delta(prev, cur):
    res = {}
    for cpu in cur:
        if cpu not in prev: continue
        a, b = prev[cpu], cur[cpu]
        idle_a = a["idle"] + a["iowait"]
        idle_b = b["idle"] + b["iowait"]
        non_a  = a["user"]+a["nice"]+a["system"]+a["irq"]+a["softirq"]+a["steal"]
        non_b  = b["user"]+b["nice"]+b["system"]+b["irq"]+b["softirq"]+b["steal"]
        totald = (idle_b - idle_a) + (non_b - non_a)
        if totald <= 0: continue
        idled  = (idle_b - idle_a)
        iowaitd= (b["iowait"] - a["iowait"])
        steald = (b["steal"]  - a["steal"])
        used   = (totald - idled)

        res[cpu] = {
            "usage": 100.0 * used / totald,
            "iowait": 100.0 * iowaitd / totald,
            "steal": 100.0 * steald / totald
        }
    return res

def main():
    # читаємо контекст (поки не використовується)
    # //_ = sys.stdin.read()

    p_stat, prun_a, pblk_a = read_proc_stat()
    time.sleep(0.2)  # 200ms — статлес-підхід без кешу
    c_stat, prun_b, pblk_b = read_proc_stat()
    load1, load5, load15 = read_loadavg()
    
    print(json.dumps({"timestamp": int(time.time())}))
    
    

    uses = usage_from_delta(p_stat, c_stat)
    metrics = []
    for cpu, v in uses.items():
        metrics.append({"name":"cpu_usage_percent","type":"gauge","value":v["usage"],"labels":{"cpu":cpu}})
        metrics.append({"name":"cpu_iowait_percent","type":"gauge","value":v["iowait"],"labels":{"cpu":cpu}})
        metrics.append({"name":"cpu_steal_percent","type":"gauge","value":v["steal"],"labels":{"cpu":cpu}})
    if prun_b is not None:
        metrics.append({"name":"procs_running","type":"gauge","value":float(prun_b),"labels":{}})
    if pblk_b is not None:
        metrics.append({"name":"procs_blocked","type":"gauge","value":float(pblk_b),"labels":{}})
    metrics += [
        {"name":"load1","type":"gauge","value":load1,"labels":{}},
        {"name":"load5","type":"gauge","value":load5,"labels":{}},
        {"name":"load15","type":"gauge","value":load15,"labels":{}},
    ]

    print(json.dumps({"metrics": metrics}))

if __name__ == "__main__":
    main()

