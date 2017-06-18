#include "../../../po.h"
#include "../../../gui/dx11/dx11_form.h"
int main()
{
	PO::context con;
	auto p = con.create_form(PO::Tmp::itself<PO::Dx11::Dx11_form>{});
	con.wait_all_form_close();
}