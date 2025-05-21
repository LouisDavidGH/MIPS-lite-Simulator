## 📄 test1.txt Instruction Breakdown
In test1.txt
00000000
040A0001
04000001
04000005
0C000005
44000000

### ✅ Summary Table

| PC   | Hex        | Instruction           | Type   | Opcode | Fields                         | Description                                |
|------|------------|------------------------|--------|--------|--------------------------------|--------------------------------------------|
| 0    | `00000000` | `ADD $0, $0, $0`       | R-type | 0      | rd=$0, rs=$0, rt=$0            | No-op                                      |
| 1    | `040A0001` | `ADDI $10, $10, 1`     | I-type | 1      | rd/rs=$10, immediate=1         | `$10 += 1`                                 |
| 2    | `04000001` | `ADDI $0, $0, 1`       | I-type | 1      | rd/rs=$0, immediate=1          | `$0 += 1`                                 |
| 3    | `04000005` | `ADDI $0, $0, 5`       | I-type | 1      | rd/rs=$0, immediate=5          | `$0 += 5`                                 |
| 4    | `0C000005` | `SUBI $0, $0, 5`       | I-type | 3      | rd/rs=$0, immediate=5          | `$0 -= 5`                                 |      
| 5    | `44000000` | `HALT`                 | N/A    | 17     | all fields = 0                 | Stop execution                            |

> ℹ️ **Notes:**
> - Register `$0` is hardwired to `0` in MIPS — writes to it are ignored.
> - Opcodes are based on your custom instruction set:
>   - `0x00` → `ADD`
>   - `0x01` → `ADDI`
>   - `0x03` → `SUBI`
>   - `0x11` → `HALT`