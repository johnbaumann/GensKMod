

	; SegaCD extended Read Byte
	; *******************************************

	ALIGN32

	M68K_Read_Byte_Bios_CD:
		cmp ebx, 0x1FFFF
		ja short .Bank_RAM

		xor ebx, 1
		mov al, [Rom_Data + ebx]
		pop ebx
		ret

	ALIGN4

	.Bank_RAM
		cmp ebx, 0x3FFFF
		ja near M68K_Read_Byte_Bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je near M68K_Read_Byte_Bad

		xor ebx, 1
		mov al, [Ram_Prg + ebx - 0x20000]		
		pop ebx
		ret

	ALIGN32
	
	M68K_Read_Byte_WRam:
		cmp ebx, 0x23FFFF
		mov eax, [Ram_Word_State]
		ja short .bad
		and eax, 0x3
		jmp [.Table_Word_Ram + eax * 4]

	ALIGN4

	.Table_Word_Ram
;		dd .Word_Ram_2M, .bad
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		xor ebx, 1
		mov al, [Ram_Word_2M + ebx - 0x200000]
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0

		xor ebx, 1
		mov al, [Ram_Word_1M + ebx - 0x200000]
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		xor ebx, 1
		mov al, [Ram_Word_1M + ebx - 0x200000 + 0x20000]
		pop ebx
		ret

	ALIGN4

	.bad
		mov al, 0
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_0
		shr ebx, 1
		mov eax, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc eax, 0
		and ebx, 0xFFFF
		mov al, [Ram_Word_1M + ebx * 2 + eax]
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_1
		shr ebx, 1
		mov eax, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc eax, 0
		and ebx, 0xFFFF
		mov al, [Ram_Word_1M + ebx * 2 + eax + 0x20000]
		pop ebx
		ret

	ALIGN32

	M68K_Read_Byte_BRAM_L:
		cmp ebx, 0x400001
		mov al, 0
		jne short .bad

		mov al, [BRAM_Ex_Size]

	.bad
		pop ebx
		ret


	ALIGN32

	M68K_Read_Byte_BRAM:
		cmp ebx, 0x61FFFF
		mov al, 0
		ja short .bad

		test word [BRAM_Ex_State], 0x100
		jz short .bad

		and ebx, 0x1FFFF
		shr ebx, 1
		mov al, [Ram_Backup_Ex + ebx]

	.bad
		pop ebx
		ret


	ALIGN32

	M68K_Read_Byte_BRAM_W:
		cmp ebx, 0x7FFFFF
		mov al, 0
		jne short .bad

		mov al, [BRAM_Ex_State]

	.bad
		pop ebx
		ret


	ALIGN32
	
	M68K_Read_Byte_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz near .bad
		
		push ecx
		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop edx
		pop ecx
		pop ebx
		ret
		
	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne short .no_busreq
		
		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call _main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_SEGACD
		ja short .bus_taken

		mov al, [Last_BUS_REQ_St]
		pop ebx
		or al, 0x80
		ret

	ALIGN4

	.bus_taken
		mov al, 0x80
		pop ebx
		ret

	ALIGN4

	.z80_on
		mov al, 0x81
		pop ebx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA1000D
		ja short .CD_Reg

		and ebx, 0x00000E
		jmp [.Table_IO_RB + ebx * 2]

	ALIGN4

	.Table_IO_RB
		dd .MD_Spec, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4
	
	.bad
		mov al, 0
		pop ebx
		ret

	ALIGN4

	.MD_Spec
		mov al, [Game_Mode]
		add al, al
		or al, [CPU_Mode]
		shl al, 6
		pop ebx
		or al, [Gen_Version]
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop ebx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov al, [Controller_1_COM]
		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov al, [Controller_2_COM]
		pop ebx
		ret

	ALIGN4

	.CD_Reg
		cmp ebx, 0xA12000
		jb short .bad
		cmp ebx, 0xA1202F
		ja short .bad

		and ebx, 0x3F
		jmp [.Table_Extended_IO + ebx * 4]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl_H, .S68K_Ctrl_L, .Memory_Ctrl_H, .Memory_Ctrl_L
		dd .CDC_Mode_H, .CDC_Mode_L, .HINT_Vector_H, .HINT_Vector_L
		dd .CDC_Host_Data_H, .CDC_Host_Data_L, .Unknow, .Unknow
		dd .Stop_Watch_H, .Stop_Watch_L, .Com_Flag_H, .Com_Flag_L
		dd .Com_D0_H, .Com_D0_L, .Com_D1_H, .Com_D1_L
		dd .Com_D2_H, .Com_D2_L, .Com_D3_H, .Com_D3_L
		dd .Com_D4_H, .Com_D4_L, .Com_D5_H, .Com_D5_L
		dd .Com_D6_H, .Com_D6_L, .Com_D7_H, .Com_D7_L
		dd .Com_S0_H, .Com_S0_L, .Com_S1_H, .Com_S1_L
		dd .Com_S2_H, .Com_S2_L, .Com_S3_H, .Com_S3_L
		dd .Com_S4_H, .Com_S4_L, .Com_S5_H, .Com_S5_L
		dd .Com_S6_H, .Com_S6_L, .Com_S7_H, .Com_S7_L

	ALIGN4

	.S68K_Ctrl_L
		mov al, [S68K_State]
		pop ebx
		ret

	ALIGN4
	
	.S68K_Ctrl_H
		mov al, [Int_Mask_S68K]
		and al, 4
		shl al, 5
		pop ebx
		ret

	ALIGN4
	
	.Memory_Ctrl_L
		mov eax, [Bank_M68K]
		mov ebx, [Ram_Word_State]
		shr eax, 11
		and ebx, 3
		or al, [Memory_Control_Status + ebx]
		pop ebx
		ret

	ALIGN4
	
	.Memory_Ctrl_H
		mov al, [S68K_Mem_WP]
		pop ebx
		ret

	ALIGN4

	.CDC_Mode_H
		mov al, [CDC.RS0 + 1]
		pop ebx
		ret

	ALIGN4

	.CDC_Mode_L
		mov al, 0
		pop ebx
		ret

	ALIGN4

	.HINT_Vector_H
		mov al, [Rom_Data + 0x73]
		pop ebx
		ret

	ALIGN4

	.HINT_Vector_L
		mov al, [Rom_Data + 0x72]
		pop ebx
		ret

	ALIGN4

	.CDC_Host_Data_H
		xor al, al
		pop ebx
		ret
		
	ALIGN4

	.CDC_Host_Data_L
		xor al, al
		pop ebx
		ret

	ALIGN4

	.Unknow
		mov al, 0
		pop ebx
		ret

	ALIGN4

	.Stop_Watch_H
		mov al, [CDC.Stop_Watch + 3]
		pop ebx
		ret

	ALIGN4

	.Stop_Watch_L
		mov al, [CDC.Stop_Watch + 2]
		pop ebx
		ret

	ALIGN4

	.Com_Flag_H
		mov al, [COMM.Flag + 1]
		pop ebx
		ret

	ALIGN4

	.Com_Flag_L
		mov al, [COMM.Flag]
		pop ebx
		ret

	ALIGN4

	.Com_D0_H
	.Com_D0_L
	.Com_D1_H
	.Com_D1_L
	.Com_D2_H
	.Com_D2_L
	.Com_D3_H
	.Com_D3_L
	.Com_D4_H
	.Com_D4_L
	.Com_D5_H
	.Com_D5_L
	.Com_D6_H
	.Com_D6_L
	.Com_D7_H
	.Com_D7_L
		xor ebx, 1
		mov al, [COMM.Command + ebx - 0x10]
		pop ebx
		ret

	ALIGN4

	.Com_S0_H
	.Com_S0_L
	.Com_S1_H
	.Com_S1_L
	.Com_S2_H
	.Com_S2_L
	.Com_S3_H
	.Com_S3_L
	.Com_S4_H
	.Com_S4_L
	.Com_S5_H
	.Com_S5_L
	.Com_S6_H
	.Com_S6_L
	.Com_S7_H
	.Com_S7_L
		xor ebx, 1
		mov al, [COMM.Status + ebx - 0x20]
		pop ebx
		ret


	; SegaCD extended Read Word
	; *******************************************

	ALIGN32

	M68K_Read_Word_Bios_CD:
		cmp ebx, 0x1FFFF
		ja short .Bank_RAM

		mov ax, [Rom_Data + ebx]
		pop ebx
		ret

	ALIGN4

	.Bank_RAM
		cmp ebx, 0x3FFFF
		ja near M68K_Read_Word_Bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je near M68K_Read_Byte_Bad

		mov ax, [Ram_Prg + ebx - 0x20000]
		pop ebx
		ret

	ALIGN32
	
	M68K_Read_Word_WRam:
		cmp ebx, 0x23FFFF
		mov eax, [Ram_Word_State]
		ja short .bad
		and eax, 0x3
		jmp [.Table_Word_Ram + eax * 4]

	ALIGN4

	.Table_Word_Ram
;		dd .Word_Ram_2M, .bad
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		mov ax, [Ram_Word_2M + ebx - 0x200000]
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0

		mov ax, [Ram_Word_1M + ebx - 0x200000]
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		mov ax, [Ram_Word_1M + ebx - 0x200000 + 0x20000]
		pop ebx
		ret

	ALIGN4

	.bad
		mov ax, 0
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_0
		xor eax, eax
		mov ax, [Cell_Conv_Tab + ebx - 0x220000]
		mov ax, [Ram_Word_1M + eax * 2]
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_1
		xor eax, eax
		mov ax, [Cell_Conv_Tab + ebx - 0x220000]
		mov ax, [Ram_Word_1M + eax * 2 + 0x20000]
		pop ebx
		ret


	ALIGN32

	M68K_Read_Word_BRAM_L:
		cmp ebx, 0x400000
		mov ax, 0
		jne short .bad

		mov ax, [BRAM_Ex_Size]

	.bad
		pop ebx
		ret


	ALIGN32

	M68K_Read_Word_BRAM:
		cmp ebx, 0x61FFFF
		mov ax, 0
		ja short .bad

		test word [BRAM_Ex_State], 0x100
		jz short .bad

		and ebx, 0x1FFFF
		shr ebx, 1
		mov ax, [Ram_Backup_Ex + ebx]

	.bad
		pop ebx
		ret


	ALIGN32

	M68K_Read_Word_BRAM_W:
		cmp ebx, 0x7FFFFE
		mov ax, 0
		jne short .bad

		xor ah, ah
		mov al, [BRAM_Ex_State]

	.bad
		pop ebx
		ret


	ALIGN32
	
	M68K_Read_Word_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram
		
		test byte [Z80_State], 6
		jnz near .bad

		push ecx
		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		call [Z80_ReadB_Table + ebx]
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_Z80_ram
		cmp ebx, 0xA11100
		jne short .no_busreq
		
		test byte [Z80_State], 2
		jnz short .z80_on

	.z80_off
		call _main68k_readOdometer
		sub eax, [Last_BUS_REQ_Cnt]
		cmp eax, CYCLE_FOR_TAKE_Z80_BUS_SEGACD
		ja short .bus_taken

		mov al, [Fake_Fetch]
		mov ah, [Last_BUS_REQ_St]
		xor al, 0xFF
		or ah, 0x80
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		pop ebx
		ret

	ALIGN4

	.bus_taken
		mov al, [Fake_Fetch]
		mov ah, 0x80
		xor al, 0xFF
		pop ebx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.z80_on
		mov al, [Fake_Fetch]
		mov ah, 0x81
		xor al, 0xFF
		pop ebx
		mov [Fake_Fetch], al				; fake the next fetched instruction (random)
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA1000D
		ja short .CD_Reg

		and ebx, 0x00000E
		jmp [.Table_IO_RW + ebx * 2]

	ALIGN4

	.Table_IO_RW
		dd .MD_Spec, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.MD_Spec
		mov ax, [Game_Mode]
		add ax, ax
		or ax, [CPU_Mode]
		shl ax, 6
		pop ebx
		or ax, [Gen_Version]									; on recupere les infos hardware de la machine
		ret

	ALIGN4

	.Pad_1
		call RD_Controller_1
		pop ebx
		ret

	ALIGN4

	.Pad_2
		call RD_Controller_2
		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov ax, [Controller_1_COM]
		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov ax, [Controller_2_COM]
		pop ebx
		ret

	ALIGN4

	.bad
		xor ax, ax
		pop ebx
		ret

	ALIGN4

	.CD_Reg
		cmp ebx, 0xA12000
		jb short .bad
		cmp ebx, 0xA1202F
		ja short .bad

		and ebx, 0x3E
		jmp [.Table_Extended_IO + ebx * 2]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl, .Memory_Ctrl, .CDC_Mode, .HINT_Vector
		dd .CDC_Host_Data, .Unknow, .Stop_Watch, .Com_Flag
		dd .Com_D0, .Com_D1, .Com_D2, .Com_D3
		dd .Com_D4, .Com_D5, .Com_D6, .Com_D7
		dd .Com_S0, .Com_S1, .Com_S2, .Com_S3
		dd .Com_S4, .Com_S5, .Com_S6, .Com_S7

	ALIGN4

	.S68K_Ctrl
		mov ah, [Int_Mask_S68K]
		mov al, [S68K_State]
		and ah, 4
		shl ah, 5
		pop ebx
		ret

	ALIGN4
	
	.Memory_Ctrl
		mov eax, [Bank_M68K]
		mov ebx, [Ram_Word_State]
		shr eax, 11
		and ebx, 3
		mov ah, [S68K_Mem_WP]
		or al, [Memory_Control_Status + ebx]
		pop ebx
		ret

	ALIGN4

	.CDC_Mode
		mov ah, [CDC.RS0 + 1]
		mov al, 0
		pop ebx
		ret

	ALIGN4

	.HINT_Vector
		mov ax, [Rom_Data + 0x72]
		pop ebx
		ret

	ALIGN4

	.CDC_Host_Data
		call _Read_CDC_Host_MAIN
		pop ebx
		ret

	ALIGN4

	.Unknow
		mov ax, 0
		pop ebx
		ret

	ALIGN4

	.Stop_Watch
		mov ax, [CDC.Stop_Watch + 2]
		pop ebx
		ret

	ALIGN4

	.Com_Flag
		mov ax, [COMM.Flag]
		pop ebx
		ret

	ALIGN4

	.Com_D0
	.Com_D1
	.Com_D2
	.Com_D3
	.Com_D4
	.Com_D5
	.Com_D6
	.Com_D7
		mov ax, [COMM.Command + ebx - 0x10]
		pop ebx
		ret

	ALIGN4

	.Com_S0
	.Com_S1
	.Com_S2
	.Com_S3
	.Com_S4
	.Com_S5
	.Com_S6
	.Com_S7
		mov ax, [COMM.Status + ebx - 0x20]
		pop ebx
		ret


	; SegaCD extended Write Byte
	; *******************************************

	ALIGN32
	
	M68K_Write_Byte_Bios_CD:
		cmp ebx, 0x20000
		jb short .bad
		cmp ebx, 0x3FFFF
		ja short .bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je short .bad

		xor ebx, 1
		mov [Ram_Prg + ebx - 0x20000], al

	.bad
		pop ecx
		pop ebx
		ret

	ALIGN32

	M68K_Write_Byte_WRam:
		cmp ebx, 0x23FFFF
		mov ecx, [Ram_Word_State]
		ja short .bad
		and ecx, 0x3
		jmp [.Table_Word_Ram + ecx * 4]

	ALIGN4

	.Table_Word_Ram
;		dd .Word_Ram_2M, .bad
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		xor ebx, 1
		mov [Ram_Word_2M + ebx - 0x200000], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0
		xor ebx, 1
		mov [Ram_Word_1M + ebx - 0x200000], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		xor ebx, 1
		mov [Ram_Word_1M + ebx - 0x200000 + 0x20000], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.bad
		mov ax, 0
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_0
		shr ebx, 1
		mov ecx, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc ecx, 0
		and ebx, 0xFFFF
		mov [Ram_Word_1M + ebx * 2 + ecx], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_1
		shr ebx, 1
		mov ecx, 0
		mov bx, [Cell_Conv_Tab + ebx * 2 - 0x220000]
		adc ecx, 0
		and ebx, 0xFFFF
		mov [Ram_Word_1M + ebx * 2 + ecx + 0x20000], al
		pop ecx
		pop ebx
		ret


	ALIGN32

	M68K_Write_Byte_BRAM:
		cmp ebx, 0x61FFFF
		ja short .bad

		cmp word [BRAM_Ex_State], 0x101
		jne short .bad

		and ebx, 0x1FFFF
		shr ebx, 1
		mov [Ram_Backup_Ex + ebx], al

	.bad
		pop ecx
		pop ebx
		ret


	ALIGN32

	M68K_Write_Byte_BRAM_W:
		cmp ebx, 0x7FFFFF
		jne short .bad

		mov [BRAM_Ex_State], al

	.bad
		pop ecx
		pop ebx
		ret


	ALIGN32
	
	M68K_Write_Byte_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_mem

		test byte [Z80_State], 6
		jnz short .bad

		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		shr ebx, 10
		mov edx, eax
		call [Z80_WriteB_Table + ebx]
		pop edx
		pop ecx
		pop ebx
		ret
		
	ALIGN4

	.bad
		pop ecx
		pop ebx
		ret

	ALIGN4

	.no_Z80_mem
		cmp ebx, 0xA11100
		jne near .no_busreq

		xor ecx, ecx
		mov ah, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test al, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .desactived

		test ah, 2
		jnz short .already_actived

		or ah, 2
		push edx
		mov [Z80_State], ah
		mov ebx, [Cycles_M68K]
		call _main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop edx

	.already_actived
		pop ecx
		pop ebx
		ret

	ALIGN4

	.desactived
		call _main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_desactived

		push edx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop edx

	.already_desactived
		pop ecx
		pop ebx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test al, 1
		jnz short .no_reset

		push edx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call _YM2612_Reset
		pop edx
		pop ecx
		pop ebx
		ret

	.no_reset
		and byte [Z80_State], ~4
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_reset_z80
		cmp ebx, 0xA1000D
		ja short .no_ctrl_io

		and ebx, 0x00000E
		jmp [.Table_IO_WB + ebx * 2]

	ALIGN4

	.Table_IO_WB
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push eax
		call WR_Controller_1
		add esp, 4
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Pad_2
		push eax
		call WR_Controller_2
		add esp, 4
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], al
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_ctrl_io
		cmp ebx, 0xA12000
		jb near M68K_Write_Byte_Bad
		cmp ebx, 0xA1202F
		ja near M68K_Write_Byte_Bad

		and ebx, 0x3F
		jmp [.Table_Extended_IO + ebx * 4]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl_H, .S68K_Ctrl_L, .Memory_Ctrl_H, .Memory_Ctrl_L
		dd .CDC_Mode_H, .CDC_Mode_L, .HINT_Vector_H, .HINT_Vector_L
		dd .CDC_Host_Data_H, .CDC_Host_Data_L, .Unknow, .Unknow
		dd .Stop_Watch_H, .Stop_Watch_L, .Com_Flag_H, .Com_Flag_L
		dd .Com_D0_H, .Com_D0_L, .Com_D1_H, .Com_D1_L
		dd .Com_D2_H, .Com_D2_L, .Com_D3_H, .Com_D3_L
		dd .Com_D4_H, .Com_D4_L, .Com_D5_H, .Com_D5_L
		dd .Com_D6_H, .Com_D6_L, .Com_D7_H, .Com_D7_L
		dd .Com_S0_H, .Com_S0_L, .Com_S1_H, .Com_S1_L
		dd .Com_S2_H, .Com_S2_L, .Com_S3_H, .Com_S3_L
		dd .Com_S4_H, .Com_S4_L, .Com_S5_H, .Com_S5_L
		dd .Com_S6_H, .Com_S6_L, .Com_S7_H, .Com_S7_L

	ALIGN4

	.S68K_Ctrl_L
		test al, 1
		jz short .S68K_Reseting
		test byte [S68K_State], 1
		jnz short .S68K_Already_Running

	.S68K_Restart
	
		push eax
		call _sub68k_reset
		pop eax

	.S68K_Reseting
	.S68K_Already_Running
		and al, 3
		mov [S68K_State], al
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.S68K_Ctrl_H
		test al, 0x1
		jz .No_Process_INT2
		test byte [Int_Mask_S68K], 0x4
		jz .No_Process_INT2

		push dword -1
		push dword 2
		call _sub68k_interrupt
		add esp, 8

	.No_Process_INT2
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.Memory_Ctrl_L
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		mov ebx, eax
		shr eax, 1
		and ebx, 0xC0
		test byte [Ram_Word_State], 0x2
		jnz short .Mode_1M

	.Mode_2M
		shl ebx, 11
		test al, 1
		mov [Bank_M68K], ebx
		jz short .No_DMNA

		mov byte [Ram_Word_State], 1
		call _MS68K_Set_Word_Ram

	.No_DMNA
		call _M68K_Set_Prg_Ram
		pop ecx
		pop ebx
		ret
		
	ALIGN4

	.Mode_1M
		shl ebx, 11
		test al, 1
		jnz short .no_swap

		or word [Memory_Control_Status + 2], 0x0202		; DMNA bit = 1

	.no_swap
		mov [Bank_M68K], ebx
		call _M68K_Set_Prg_Ram
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.Memory_Ctrl_H
		mov [S68K_Mem_WP], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CDC_Mode_H
	.CDC_Mode_L
		pop ecx
		pop ebx
		ret

	ALIGN4

	.HINT_Vector_H
		mov [Rom_Data + 0x73], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.HINT_Vector_L
		mov [Rom_Data + 0x72], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CDC_Host_Data_H
	.CDC_Host_Data_L
	.Unknow
	.Stop_Watch_H
	.Stop_Watch_L
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_Flag_H
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		mov [COMM.Flag + 1], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_Flag_L
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		rol al, 1
		mov byte [COMM.Flag + 1], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_D0_H
	.Com_D0_L
	.Com_D1_H
	.Com_D1_L
	.Com_D2_H
	.Com_D2_L
	.Com_D3_H
	.Com_D3_L
	.Com_D4_H
	.Com_D4_L
	.Com_D5_H
	.Com_D5_L
	.Com_D6_H
	.Com_D6_L
	.Com_D7_H
	.Com_D7_L
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		xor ebx, 1
		mov [COMM.Command + ebx - 0x10], al
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_S0_H
	.Com_S0_L
	.Com_S1_H
	.Com_S1_L
	.Com_S2_H
	.Com_S2_L
	.Com_S3_H
	.Com_S3_L
	.Com_S4_H
	.Com_S4_L
	.Com_S5_H
	.Com_S5_L
	.Com_S6_H
	.Com_S6_L
	.Com_S7_H
	.Com_S7_L
		pop ecx
		pop ebx
		ret


	; SegaCD extended Write Word
	; *******************************************

	ALIGN32
	
	M68K_Write_Word_Bios_CD:
		cmp ebx, 0x20000
		jb short .bad
		cmp ebx, 0x3FFFF
		ja short .bad

		add ebx, [Bank_M68K]
		cmp byte [S68K_State], 1			; BUS available ?
		je short .bad

		mov [Ram_Prg + ebx - 0x20000], ax

	.bad
		pop ecx
		pop ebx
		ret

	ALIGN32
	
	M68K_Write_Word_WRam:
		cmp ebx, 0x23FFFF
		mov ecx, [Ram_Word_State]
		ja short .bad
		and ecx, 0x3
		jmp [.Table_Word_Ram + ecx * 4]

	ALIGN4

	.Table_Word_Ram
		dd .Word_Ram_2M, .Word_Ram_2M
		dd .Word_Ram_1M_0, .Word_Ram_1M_1

	ALIGN4

	.Word_Ram_2M
		mov [Ram_Word_2M + ebx - 0x200000], ax
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_0
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_0

		mov [Ram_Word_1M + ebx - 0x200000], ax
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Word_Ram_1M_1
		cmp ebx, 0x21FFFF
		ja short .Cell_Arranged_1

		mov [Ram_Word_1M + ebx  - 0x200000 + 0x20000], ax
		pop ecx
		pop ebx
		ret

	ALIGN4

	.bad
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_0
		mov bx, [Cell_Conv_Tab + ebx - 0x220000]
		and ebx, 0xFFFF
		pop ecx
		mov [Ram_Word_1M + ebx * 2], ax
		pop ebx
		ret

	ALIGN4

	.Cell_Arranged_1
		mov bx, [Cell_Conv_Tab + ebx - 0x220000]
		and ebx, 0xFFFF
		pop ecx
		mov [Ram_Word_1M + ebx * 2 + 0x20000], ax
		pop ebx
		ret


	ALIGN32

	M68K_Write_Word_BRAM:
		cmp ebx, 0x61FFFF
		ja short .bad

		cmp word [BRAM_Ex_State], 0x101
		jne short .bad

		and ebx, 0x1FFFE
		shr ebx, 1
		mov [Ram_Backup_Ex + ebx], ax

	.bad
		pop ecx
		pop ebx
		ret


	ALIGN32

	M68K_Write_Word_BRAM_W:
		cmp ebx, 0x7FFFFE
		jne short .bad

		mov [BRAM_Ex_State], al

	.bad
		pop ecx
		pop ebx
		ret


	ALIGN32
	
	M68K_Write_Word_Misc_CD:
		cmp ebx, 0xA0FFFF
		ja short .no_Z80_ram
		
		test byte [Z80_State], 6
		jnz near .bad

		push edx
		mov ecx, ebx
		and ebx, 0x7000
		and ecx, 0x7FFF
		mov dh, al
		shr ebx, 10
		mov dl, al
		call [Z80_WriteB_Table + ebx]
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_Z80_ram
		cmp ebx, 0xA11100
		jne near .no_busreq
		
		xor ecx, ecx
		mov al, [Z80_State]
		mov dword [Controller_1_Counter], ecx
		test ah, 1
		mov dword [Controller_1_Delay], ecx
		mov dword [Controller_2_Counter], ecx
		mov dword [Controller_2_Delay], ecx
		jnz short .desactived

		test al, 2
		jnz short .already_actived

		or al, 2
		push edx
		mov [Z80_State], al
		mov ebx, [Cycles_M68K]
		call _main68k_readOdometer
		sub ebx, eax
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Set_Odo
		pop edx

	.already_actived
		pop ecx
		pop ebx
		ret

	ALIGN4

	.desactived
		call _main68k_readOdometer
		mov cl, [Z80_State]
		mov [Last_BUS_REQ_Cnt], eax
		test cl, 2
		setnz [Last_BUS_REQ_St]
		jz short .already_desactived

		push edx
		mov ebx, [Cycles_M68K]
		and cl, ~2
		sub ebx, eax
		mov [Z80_State], cl
		mov edx, [Cycles_Z80]
		mov ebx, [Z80_M68K_Cycle_Tab + ebx * 4]
		mov ecx, M_Z80
		sub edx, ebx
		call z80_Exec
		pop edx

	.already_desactived
		pop ecx
		pop ebx
		ret

	ALIGN4

	.no_busreq
		cmp ebx, 0xA11200
		jne short .no_reset_z80

		test ah, 1
		jnz short .no_reset

		push edx
		mov ecx, M_Z80
		call z80_Reset
		or byte [Z80_State], 4
		call _YM2612_Reset
		pop edx
		pop ecx
		pop ebx
		ret

	.no_reset
		and byte [Z80_State], ~4
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_reset_z80
		cmp ebx, 0xA1000D
		ja short .no_ctrl_io

		and ebx, 0x00000E
		jmp [.Table_IO_WW + ebx * 2]

	ALIGN4

	.Table_IO_WW
		dd .bad, .Pad_1, .Pad_2, .bad
		dd .CT_Pad_1, .CT_Pad_2, .bad, .bad

	ALIGN4

	.Pad_1
		push eax
		call WR_Controller_1
		add esp, 4
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Pad_2
		push eax
		call WR_Controller_2
		add esp, 4
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CT_Pad_1
		mov [Controller_1_COM], ax
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CT_Pad_2
		mov [Controller_2_COM], ax
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.bad
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.no_ctrl_io
		cmp ebx, 0xA12000
		jb short .bad
		cmp ebx, 0xA1202F
		ja short .bad

		and ebx, 0x3E
		jmp [.Table_Extended_IO + ebx * 2]

	ALIGN4

	.Table_Extended_IO
		dd .S68K_Ctrl, .Memory_Ctrl, .CDC_Mode, .HINT_Vector
		dd .CDC_Host_Data, .Unknow, .Stop_Watch, .Com_Flag
		dd .Com_D0, .Com_D1, .Com_D2, .Com_D3
		dd .Com_D4, .Com_D5, .Com_D6, .Com_D7
		dd .Com_S0, .Com_S1, .Com_S2, .Com_S3
		dd .Com_S4, .Com_S5, .Com_S6, .Com_S7

	ALIGN4

	.S68K_Ctrl
		test al, 1
		jz short .S68K_Reseting
		test byte [S68K_State], 1
		jnz short .S68K_Already_Running

	.S68K_Restart
	
		push eax
		call _sub68k_reset
		pop eax

	.S68K_Reseting
	.S68K_Already_Running
		and al, 3
		test ah, 1
		mov [S68K_State], al
		jz short .No_Process_INT2
		test byte [Int_Mask_S68K], 0x4
		jz short .No_Process_INT2

		push dword -1
		push dword 2
		call _sub68k_interrupt
		add esp, 8

	.No_Process_INT2
		pop ecx
		pop ebx
		ret

	ALIGN4
	
	.Memory_Ctrl
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		mov [S68K_Mem_WP], ah
		mov ebx, eax
		shr eax, 1
		and ebx, 0xC0
		test byte [Ram_Word_State], 0x2
		jnz short .Mode_1M

	.Mode_2M
		shl ebx, 11
		test al, 1
		mov [Bank_M68K], ebx
		jz short .No_DMNA

		mov byte [Ram_Word_State], 1
		call _MS68K_Set_Word_Ram

	.No_DMNA
		call _M68K_Set_Prg_Ram
		pop ecx
		pop ebx
		ret
		
	ALIGN4
	
	.Mode_1M
		shl ebx, 11
		test al, 1
		jnz short .no_swap

		or word [Memory_Control_Status + 2], 0x0202		; DMNA bit = 1

	.no_swap
		mov [Bank_M68K], ebx
		call _M68K_Set_Prg_Ram
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CDC_Mode
		pop ecx
		pop ebx
		ret

	ALIGN4

	.HINT_Vector
		mov [Rom_Data + 0x72], ax
		pop ecx
		pop ebx
		ret

	ALIGN4

	.CDC_Host_Data
	.Unknow
	.Stop_Watch
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_Flag
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		mov [COMM.Flag + 1], ah
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_D0
	.Com_D1
	.Com_D2
	.Com_D3
	.Com_D4
	.Com_D5
	.Com_D6
	.Com_D7
;pushad
;push eax
;push ebx
;add dword [esp], 0xA12000
;call _Write_To_68K_Space
;pop ebx
;pop eax
;popad

		mov [COMM.Command + ebx - 0x10], ax
		pop ecx
		pop ebx
		ret

	ALIGN4

	.Com_S0
	.Com_S1
	.Com_S2
	.Com_S3
	.Com_S4
	.Com_S5
	.Com_S6
	.Com_S7
		pop ecx
		pop ebx
		ret
