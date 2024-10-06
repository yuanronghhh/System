from datetime import datetime
import json
import decimal


class JsonEncoder(json.JSONEncoder):
    def default(self, o): # pylint: disable=E0202

        if isinstance(o, set):
            return list(o)

        if isinstance(o, datetime):
            return o.isoformat()

        if isinstance(o, bytes):
            if o == b'\x00':
                return False
            elif o == b'\x01':
                return True

        if isinstance(o, decimal.Decimal):
            return float(o)      #  这里会影响到model取出数据

        if isinstance(o, object):
            return o.__dict__

        return json.JSONEncoder.default(self, o)

class Serializer:

    @staticmethod
    def serialize(obj):
        if obj is None:
            return

        return json.dumps(obj, cls=JsonEncoder, ensure_ascii=False)

    @staticmethod
    def deserialize(jstr):
        if not jstr:
            return

        return json.loads(jstr)

    @staticmethod
    def deserialize_result(ls, keys):
        if not ls:
            return []

        if isinstance(ls, list):
            for l in ls:
                for k in keys:
                    if k in l:
                        l[k] = Serializer.deserialize(l[k])

            return ls

        elif isinstance(ls, dict):
            for k in keys:
                if k in ls:
                    ls[k] = Serializer.deserialize(ls[k])

            return ls
